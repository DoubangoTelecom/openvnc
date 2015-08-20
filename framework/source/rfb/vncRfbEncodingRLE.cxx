/* Copyright (C) 2011, Mamadou Diop.
 * Copyright (c) 2011, Doubango Telecom. All rights reserved.
 *
 * Contact: Mamadou Diop <diopmamadou(at)doubango(dot)org>
 *       
 * This file is part of OpenVNC Project ( http://code.google.com/p/openvnc )
 *
 * idoubs is free software: you can redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software Foundation, either version 3 
 * of the License, or (at your option) any later version.
 *       
 * idoubs is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 * See the GNU General Public License for more details.
 *       
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "openvnc/rfb/vncRfbEncodingRLE.h"
#include "openvnc/vncUtils.h"
#include "openvnc/network/vncNetUtils.h"

#include <math.h>

struct vncRleFillTileResult_s
{
	vncError_t eError;
	unsigned nWroteBytes;
	unsigned nReadBytes;
}
vncRleFillTileResult_t;

vncRfbEncodingRLE::vncRfbEncodingRLE(vncRfbEncoding_t eEncoding)
:vncRfbEncoding(eEncoding)
{
	m_Decode.pTileData = NULL;
	m_Decode.pOutData = NULL;
	m_Decode.nOutDataSize = 0;

	m_Encode.pTileData = NULL;
	m_Encode.pTilePixels = NULL;
	m_Encode.pTilePixelsLengths = NULL;
	m_Encode.pTilePixelsIndexes = NULL;
	m_Encode.pOutData = NULL;
	m_Encode.nOutDataSize = 0;
}

vncRfbEncodingRLE::~vncRfbEncodingRLE()
{
	OVNC_SAFE_FREE(m_Decode.pTileData);
	OVNC_SAFE_FREE(m_Decode.pOutData);
	vncObjectSafeFree(m_Decode.pVNCPixelReader);

	OVNC_SAFE_FREE(m_Encode.pTileData);
	OVNC_SAFE_FREE(m_Encode.pTilePixels);
	OVNC_SAFE_FREE(m_Encode.pTilePixelsLengths);
	OVNC_SAFE_FREE(m_Encode.pTilePixelsIndexes);
	OVNC_SAFE_FREE(m_Encode.pOutData);
	vncObjectSafeFree(m_Encode.pVNCPixelReader);
	vncObjectSafeFree(m_Encode.pVNCByteWriter);
}

vncError_t vncRfbEncodingRLE::Encode(const vncObjectWrapper<vncRfbFrameBuffer*> pVNCFrameBuffer, const vncRfbRectangle_t* pInRectangle, const vncRfbPixelFormat_t* pOutFormat, const void* pOutData, unsigned &nOutDataSize)
{
	vncError_t ret = vncError_Ok;

	if(!*pVNCFrameBuffer || !pVNCFrameBuffer->GetBuffer() || !pInRectangle || !pOutFormat || !pOutData || !nOutDataSize){
		OVNC_DEBUG_ERROR("Invalid parameter");
		ret = vncError_InvalidParameter;
		goto done;
	}

	// allocate output buffer (at least raw buffer size)
	if(m_Encode.nOutDataSize < pVNCFrameBuffer->GetBufferSize()){
		if(!(m_Encode.pOutData = vncUtils::Realloc(m_Encode.pOutData, pVNCFrameBuffer->GetBufferSize()))){
			OVNC_DEBUG_ERROR("Failed to allocate buffer");
			return vncError_MemoryAllocFailed;
		}
		m_Encode.nOutDataSize = pVNCFrameBuffer->GetBufferSize();
	}

	// create byte writer
	if(!m_Encode.pVNCByteWriter && !(m_Encode.pVNCByteWriter = new vncByteWriter())){
		OVNC_DEBUG_ERROR("Failed to create vncByteWriter instance");
		return vncError_ApplicationError;
	}
	m_Encode.pVNCByteWriter->SetData(m_Encode.pOutData, m_Encode.nOutDataSize);

	// allocate tile
	if(!m_Encode.pTileData && !(m_Encode.pTileData = (vncRfbPixel_t*)vncUtils::Calloc(GetTileWidth() * GetTileHeight(), sizeof(vncRfbPixel_t)))){
		OVNC_DEBUG_ERROR("Failed to allocate buffer");
		ret = vncError_MemoryAllocFailed;
		goto done;
	}
	if(!m_Encode.pTilePixels && !(m_Encode.pTilePixels = (vncRfbPixel_t*)vncUtils::Calloc(GetTileWidth() * GetTileHeight(), sizeof(vncRfbPixel_t)))){
		OVNC_DEBUG_ERROR("Failed to allocate buffer");
		ret = vncError_MemoryAllocFailed;
		goto done;
	}
	if(!m_Encode.pTilePixelsLengths && !(m_Encode.pTilePixelsLengths = (unsigned*)vncUtils::Calloc(GetTileWidth() * GetTileHeight(), sizeof(unsigned)))){
		OVNC_DEBUG_ERROR("Failed to allocate buffer");
		ret = vncError_MemoryAllocFailed;
		goto done;
	}
	if(!m_Encode.pTilePixelsIndexes && !(m_Encode.pTilePixelsIndexes = (unsigned*)vncUtils::Calloc(GetTileWidth() * GetTileHeight(), sizeof(unsigned)))){
		OVNC_DEBUG_ERROR("Failed to allocate buffer");
		ret = vncError_MemoryAllocFailed;
		goto done;
	}

	unsigned nTileWidth = GetTileWidth();
	unsigned nTileHeight = GetTileHeight();
	unsigned nTileSize = (nTileWidth * nTileHeight);
	unsigned nRectTop = pInRectangle->y_position;
	unsigned nRectLeft = pInRectangle->x_position;
	unsigned nRectBottom = nRectTop + pInRectangle->height;
	unsigned nRectRight = nRectLeft + pInRectangle->width;

	uint8_t subencoding;	
	vncRfbPixel_t pixel;
	unsigned nPixelsLength = 0;
	bool bPixelFound = false;

	// FIXME: Reader should respect rectangle
	if(!m_Encode.pVNCPixelReader){
		m_Encode.pVNCPixelReader = vncRfbPixelReader::New(
			pVNCFrameBuffer->GetPixelFormat(), 
			pOutFormat, 
			vncRfbEncoding_Raw,
			pInRectangle->encoding_type,
			pVNCFrameBuffer->GetBuffer(), 
			pVNCFrameBuffer->GetBufferSize());
	}
	else{
		m_Encode.pVNCPixelReader->Reset(
				pVNCFrameBuffer->GetPixelFormat(),
				pOutFormat,
				vncRfbEncoding_Raw,
				pInRectangle->encoding_type,
				pVNCFrameBuffer->GetBuffer(), 
				pVNCFrameBuffer->GetBufferSize());
	}
	
	unsigned outPixelSize = m_Encode.pVNCPixelReader->GetOutPixelSize();
	unsigned inPixelSize = m_Encode.pVNCPixelReader->GetInPixelSize();
	
	register unsigned nReaderIndex, nIndex, nTileSizeInPixel;

	for(register unsigned y = nRectTop; y < nRectBottom; y += nTileHeight){
		unsigned h = OVNC_MIN(nRectBottom - y, nTileHeight);
		for(register unsigned x = nRectLeft; x < nRectRight; x += nTileWidth){
			unsigned w = OVNC_MIN(nRectRight - x, nTileWidth);
			
			nTileSizeInPixel = (h * w);

			// load pixels in tile
			nReaderIndex = y * pInRectangle->width + x;
			nIndex = 0;
			for(register unsigned _y = 0; _y < h; ++_y){
				for(register unsigned _x = 0; _x < w; ++_x){
					m_Encode.pTileData[nIndex++] = m_Encode.pVNCPixelReader->ReadPixel(nReaderIndex++ * inPixelSize);
				}
				nReaderIndex += (pInRectangle->width - w);
			}
			
			// run-lenth data
			nPixelsLength = 0;
			for(nIndex = 0; nIndex < nTileSizeInPixel; ++nIndex){
				pixel = m_Encode.pTileData[nIndex];
				bPixelFound = false;
				for(register unsigned i = 0; i < nPixelsLength; i++){
					if(m_Encode.pTilePixels[i] == pixel){
						m_Encode.pTilePixelsLengths[i] += 1;
						m_Encode.pTilePixelsIndexes[nIndex] = i;
						bPixelFound = true;
						break;
					}
				}
				if(!bPixelFound){
					m_Encode.pTilePixelsLengths[nPixelsLength] = 1;
					m_Encode.pTilePixels[nPixelsLength] = pixel;
					m_Encode.pTilePixelsIndexes[nIndex] = nPixelsLength;
					++nPixelsLength;
				}
			}
			

			// 1 - A solid tile consisting of a single colour
			if(nPixelsLength == 1){
				subencoding = 1;
				m_Encode.pVNCByteWriter->Write(subencoding, 1);
				m_Encode.pVNCByteWriter->Write(m_Encode.pTilePixels[0], outPixelSize);
			}
			// 2 to 16 - Packed palette types
			else if(nPixelsLength >= 2 && nPixelsLength <= 16){
				subencoding = nPixelsLength;
				m_Encode.pVNCByteWriter->Write(nPixelsLength, 1);
				// write pallete
				for(nIndex = 0; nIndex < nPixelsLength; ++nIndex){
					m_Encode.pVNCByteWriter->Write(m_Encode.pTilePixels[nIndex], outPixelSize);
				}
				// write pallete indexes
				uint8_t bpp;
				switch (nPixelsLength) {
                    case 2: bpp = 1; break;
                    case 3: case 4: bpp = 2; break;
                    default: bpp = 4; break;
                }
				uint8_t byte = 0x00;
				uint8_t bytes_count = 0;
				for(nIndex = 0; nIndex < nTileSizeInPixel; ++nIndex){
					unsigned index = m_Encode.pTilePixelsIndexes[nIndex];
					byte |= index << (8 - bytes_count - bpp);
					bytes_count += bpp;
					if(bytes_count == 8){
						m_Encode.pVNCByteWriter->Write(byte, 1);
						byte = 0x00;
						bytes_count = 0;
					}
				}
			}
			// FIXME:130 to 255
#if 0
			else if(nPixelsLength >= 130 && nPixelsLength <= 255){
				subencoding = (0x80 | nPixelsLength);
				m_Encode.pVNCByteWriter->Write(nPixelsLength, 1);
				// write pallete
				for(nIndex = 0; nIndex < nPixelsLength; ++nIndex){
					m_Encode.pVNCByteWriter->Write(m_Encode.pTilePixels[nIndex], outPixelSize);
				}
				// run-length
				unsigned _nIndex;
				signed _nLength;
				for(nIndex = 0; nIndex < nTileSizeInPixel; ){
					for(_nIndex = nIndex + 1, _nLength = 0; 
						(m_Encode.pTilePixelsIndexes[nIndex] == m_Encode.pTilePixelsIndexes[_nIndex] && _nIndex < nTileSizeInPixel); 
						++_nIndex, ++_nLength);
					
					// pixelValue
					pVNCByteWriter->Write(m_Encode.pTilePixels[m_Encode.pTilePixelsIndexes[nIndex]], outPixelSize);
					if(_nLength == 1){
						// palette index
						m_Encode.pVNCByteWriter->Write(m_Encode.pTilePixelsIndexes[nIndex], 1);
					}
					else{
						// palette index with the top bit set
						m_Encode.pVNCByteWriter->Write(0x80 | m_Encode.pTilePixelsIndexes[nIndex], 1);
						// length of the run as for plain RLE
						do{
							pVNCByteWriter->Write(OVNC_MIN(_nLength, 255), 1);
							_nLength -= 255;
						}while(_nLength >= 0);
					}
					
					nIndex = _nIndex;				
				}
			}
#endif
			// 128 - Plain RLE
			else if(nPixelsLength > 127){
				subencoding = 128;
				m_Encode.pVNCByteWriter->Write(subencoding, 1);
				unsigned _nIndex;
				signed _nLength;
				for(nIndex = 0; nIndex < nTileSizeInPixel; ){
					for(_nIndex = nIndex + 1, _nLength = 0; 
						(m_Encode.pTilePixelsIndexes[nIndex] == m_Encode.pTilePixelsIndexes[_nIndex] && _nIndex < nTileSizeInPixel); 
						++_nIndex, ++_nLength);
					
					// pixelValue
					m_Encode.pVNCByteWriter->Write(m_Encode.pTilePixels[m_Encode.pTilePixelsIndexes[nIndex]], outPixelSize);
					do{
						m_Encode.pVNCByteWriter->Write(OVNC_MIN(_nLength, 255), 1);
						_nLength -= 255;
					}while(_nLength >= 0);
					
					nIndex = _nIndex;				
				}
			}
			// 0- Raw pixel data
			else{
				subencoding = 0;
				m_Encode.pVNCByteWriter->Write(subencoding, 1);
				for(nIndex = 0; nIndex < nTileSizeInPixel; ++nIndex){
					m_Encode.pVNCByteWriter->Write(m_Encode.pTileData[nIndex], outPixelSize);
				}
			}
		}
	}

	// Encode
	ret = EncodeRaw(m_Encode.pVNCByteWriter->GetData(), m_Encode.pVNCByteWriter->GetDataIndex(), pOutData, nOutDataSize);

done:
	return ret;
}

vncError_t vncRfbEncodingRLE::Decode(vncObjectWrapper<vncVideoFrame*>pVNCVideoFrame, vncObjectWrapper<vncRfbFrameBuffer*> pVNCFrameBuffer)
{
	if(!pVNCVideoFrame || !pVNCVideoFrame){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}

	unsigned nOutDataSize = pVNCFrameBuffer->GetBufferSize();
	
	// allocate outData
	if(m_Decode.nOutDataSize < nOutDataSize){
		if(!(m_Decode.pOutData = vncUtils::Realloc(m_Decode.pOutData, nOutDataSize))){
			OVNC_DEBUG_ERROR("Failed to allocate buffer");
			return vncError_MemoryAllocFailed;
		}
		m_Decode.nOutDataSize = nOutDataSize;
	}
	// e.g. ZLib decompression
	vncError_t ret = DecodeCompressed(pVNCVideoFrame, m_Decode.pOutData, nOutDataSize);
	if(ret != vncError_Ok){
		OVNC_DEBUG_ERROR("Internal decompression failed");
		goto done;
	}

	if(!m_Decode.pTileData && !(m_Decode.pTileData = (vncRfbPixel_t*)vncUtils::Calloc(GetTileWidth() * GetTileHeight(), sizeof(vncRfbPixel_t)))){
		OVNC_DEBUG_ERROR("Failed to allocate buffer");
		ret = vncError_MemoryAllocFailed;
		goto done;
	}
	
	vncRfbRectangle_t* rectangle = pVNCVideoFrame->GetRectangle();

	if(!m_Decode.pVNCPixelReader){
		m_Decode.pVNCPixelReader = vncRfbPixelReader::New(
			pVNCVideoFrame->GetPixelFormat(), 
			pVNCFrameBuffer->GetPixelFormat(), 
			rectangle->encoding_type, 
			vncRfbEncoding_Raw,
			m_Decode.pOutData, 
			nOutDataSize);
	}
	else{
		m_Decode.pVNCPixelReader->Reset(
			pVNCVideoFrame->GetPixelFormat(),
			pVNCFrameBuffer->GetPixelFormat(),
			rectangle->encoding_type,
			vncRfbEncoding_Raw,
			m_Decode.pOutData,
			nOutDataSize);
	}

	if(!m_Decode.pVNCPixelReader){
		OVNC_DEBUG_ERROR("Invalid State");
		return vncError_ApplicationError;
	}

	if(m_Decode.pVNCPixelReader->GetRemainingBytes() < 1){
		OVNC_DEBUG_ERROR("Too short");
		return vncError_TooShort;
	}


	unsigned nTileWidth = GetTileWidth();
	unsigned nTileHeight = GetTileHeight();
	unsigned nTileSize = (nTileWidth * nTileHeight);
	unsigned nRectTop = rectangle->y_position;
	unsigned nRectLeft = rectangle->x_position;
	unsigned nRectBottom = nRectTop + rectangle->height;
	unsigned nRectRight = nRectLeft + rectangle->width;
	vncRfbPixel_t aPalette[128];

	for(register unsigned y = nRectTop; y < nRectBottom; y += nTileHeight){
		unsigned h = OVNC_MIN(nRectBottom - y, nTileHeight);
		for(register unsigned x = nRectLeft; x < nRectRight; x += nTileWidth){
			unsigned w = OVNC_MIN(nRectRight - x, nTileWidth);
			uint8_t subencoding = m_Decode.pVNCPixelReader->ReadByte();

			// Each tile begins with a subencoding type byte.  The top bit of this
			// byte is set if the tile has been run-length encoded, clear otherwise.
			bool bRunLength = ((subencoding & 0x80) == 0x80);

			// The bottom 7 bits indicate the size of the palette used: zero means
			// no palette, 1 means that the tile is of a single color, and 2 to 127
			// indicate a palette of that size.  The special subencoding values 129
			// and 127 indicate that the palette is to be reused from the last tile
			// that had a palette, with and without RLE, respectively.
			uint8_t nPalleteSize = (subencoding & 0x7F);
			for(uint8_t i = 0; i < nPalleteSize; ++i){
				aPalette[i] = m_Decode.pVNCPixelReader->ReadPixel();
			}
			
			// 0- Raw pixel data. width × height pixel values follow (where width and height
			// are the width and height of the tile):
			if(subencoding == 0){
				vncRfbPixel_t* _pTileData = (vncRfbPixel_t*)m_Decode.pTileData;
				for(register unsigned i = 0; i< (w * h); ++i){
					*_pTileData = m_Decode.pVNCPixelReader->ReadPixel();
					++_pTileData;
				}
			}
			// 1 - A solid tile consisting of a single colour. The pixel value follows:
			else if(subencoding == 1){
				vncRfbPixel_t* _pTileData = (vncRfbPixel_t*)m_Decode.pTileData;
				for(register unsigned i = 0; i< (w * h); ++i){
					*_pTileData = aPalette[0];
					++_pTileData;
				}
			}
			// 2 to 16 - Packed palette types. Followed by the palette, consisting of paletteSize(=
			// subencoding) pixel values. Then the packed pixels follow, each pixel represented
			// as a bit field yielding an index into the palette (0 meaning the first palette
			// 1see http://www.gzip.org/zlib/entry). 
			// For paletteSize 2, a 1-bit field is used, for paletteSize 3 or 4 a 2-bit
			// field is used and for paletteSize from 5 to 16 a 4-bit field is used. The bit fields
			// are packed into bytes, the most significant bits representing the leftmost pixel
			// (i.e. big endian). For tiles not a multiple of 8, 4 or 2 pixels wide (as appropriate),
			// padding bits are used to align each row to an exact number of bytes.
			else if(subencoding >= 2 && subencoding <= 16){
				// "paletteSize" pixel values already read and written into aPalette array
				int m;
				if(nPalleteSize <= 2) m = (int)floor(((double)w + 7.0)/8.0) * h;
				else if(nPalleteSize == 3 || nPalleteSize == 4) m = (int)floor(((double)w + 3.0)/4.0) * h;
				else m = (int)floor(((double)w + 1.0)/2.0) * h;
				int bitesPerPixel = (nPalleteSize > 16) ? 8 : 
							((nPalleteSize > 4) ? 4 : 
								((nPalleteSize > 2) ? 2 : 1)
							);
				uint8_t byte;
				vncRfbPixel_t* _pTileData = (vncRfbPixel_t*)m_Decode.pTileData;
				uint8_t mask = (0xFF >> (8 - bitesPerPixel));
				for(int i = 0; i < m; ++i){
					for(register int k = 0; k < 8; k += bitesPerPixel){
						if(k == 0){
							byte = m_Decode.pVNCPixelReader->ReadByte();
						}
						int index = (byte >> (8 - k)) & mask;
						*_pTileData = aPalette[index];
						++_pTileData;
					}
				}
			}
			// 17 to 127 - unused (no advantage over palette RLE)
			else if(subencoding >= 17 && subencoding <= 127){
				OVNC_DEBUG_ERROR("%d is invalid as subencoding", subencoding);
				return vncError_InvalidData;
			}
			// 128 - Plain RLE. Consists of a number of runs, repeated until the tile is done. Runs
			// may continue from the end of one row to the beginning of the next. Each run
			// is a represented by a single pixel value followed by the length of the run. The
			// length is represented as one or more bytes. The length is calculated as one more
			// than the sum of all the bytes representing the length. Any byte value other than
			// 255 indicates the final byte. So for example length 1 is represented as [0], 255
			// as [254], 256 as [255,0], 257 as [255,1], 510 as [255,254], 511 as [255,255,0]
			// and so on.
			else if(subencoding == 128){
				vncRfbPixel_t* _pTileData = (vncRfbPixel_t*)m_Decode.pTileData;
				vncRfbPixel_t* _pTileDataEnd = (_pTileData + (w * h));
				while(_pTileData < _pTileDataEnd){
					vncRfbPixel_t pixel = m_Decode.pVNCPixelReader->ReadPixel();
					int length = 1;
					uint8_t len;
					do{
						len = m_Decode.pVNCPixelReader->ReadByte();
						length += len;
					}while(len == 0xFF);
					
					while(length-->0 && (_pTileData < _pTileDataEnd)){
						*_pTileData = pixel;
						++_pTileData;
					}
				}
			}
			// 129 - unused
			else if(subencoding == 129){
				OVNC_DEBUG_ERROR("%d is invalid as subencoding", subencoding);
				return vncError_InvalidData;
			}
			// 130 to 255 - Palette RLE. Followed by the palette, consisting of paletteSize =
			// (subencoding - 128) pixel values:
			else if(subencoding >= 130 && subencoding <= 255){
				vncRfbPixel_t* _pTileData = (vncRfbPixel_t*)m_Decode.pTileData;
				vncRfbPixel_t* _pTileDataEnd = (_pTileData + (w * h));
				while(_pTileData < _pTileDataEnd){
					uint8_t paletteIndex = m_Decode.pVNCPixelReader->ReadByte();
					int length = 1;
					uint8_t len;
					if((paletteIndex & 0x80)){
						do{
							len = m_Decode.pVNCPixelReader->ReadByte();
							length += len;
						} while(len == 0xFF);
					}
					
					paletteIndex &= 0x7F;

					while(length-->0 && (_pTileData < _pTileDataEnd)){
						*_pTileData = aPalette[paletteIndex];
						++_pTileData;
					}
				}
			}

			vncRfbRectangle_t tileRect = { x, y, w, h, vncRfbEncoding_Raw };
			pVNCFrameBuffer->WritePixels(m_Decode.pVNCPixelReader->GetOutPixelFormat(), m_Decode.pTileData, (w * h), &tileRect);
		}
	}

done:
	return ret;
}

