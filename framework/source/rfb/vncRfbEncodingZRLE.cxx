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
#include "openvnc/rfb/vncRfbEncodingZRLE.h"

vncRfbEncodingZRLE::vncRfbEncodingZRLE()
:vncRfbEncodingRLE(vncRfbEncoding_ZRLE)
{
	m_Decoder.initialized = false;
	memset(&m_Decoder.zs, 0, sizeof(m_Decoder.zs));
	m_Decoder.zs.data_type = Z_BINARY;

	m_Encoder.initialized = false;
	memset(&m_Encoder.zs, 0, sizeof(m_Encoder.zs));
	m_Encoder.zs.data_type = Z_BINARY;
}

vncRfbEncodingZRLE::~vncRfbEncodingZRLE()
{
	if(m_Decoder.initialized){
		inflateEnd(&m_Decoder.zs);
	}
	if(m_Encoder.initialized){
		deflateEnd(&m_Encoder.zs);
	}
}

vncError_t vncRfbEncodingZRLE::DecodeCompressed(vncObjectWrapper<vncVideoFrame*>pVNCVideoFrame, const void* pOutData, unsigned &nOutDataSize)
{
	if(!pVNCVideoFrame || !pVNCVideoFrame->GetData() || !pOutData || !nOutDataSize){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	int zRet;
	m_Decoder.zs.next_in = (Bytef*)pVNCVideoFrame->GetData()->GetData();
	m_Decoder.zs.avail_in = 0;
	m_Decoder.zs.next_out = (Bytef*)pOutData;
	m_Decoder.zs.avail_out = nOutDataSize;
	if(!m_Decoder.initialized){
		if((zRet = inflateInit(&m_Decoder.zs)) != Z_OK){
			OVNC_DEBUG_ERROR("inflateInit() failed %d", zRet);
			return vncError_3rdApplicationError;
		}
		m_Decoder.initialized = true;
	}

	m_Decoder.zs.avail_in = pVNCVideoFrame->GetData()->GetDataSize();

	if((zRet = inflate(&m_Decoder.zs, Z_SYNC_FLUSH)) != Z_OK){
		OVNC_DEBUG_ERROR("inflate() failed %d", zRet);
		return vncError_3rdApplicationError;
	}

	if(m_Decoder.zs.avail_out <= 0){
		OVNC_DEBUG_ERROR("BufferOverflow");
		return vncError_3rdApplicationError;
	}

	nOutDataSize -= m_Decoder.zs.avail_out;
	return vncError_Ok;
}

vncError_t vncRfbEncodingZRLE::EncodeRaw(const void* pInData, unsigned nInDataSize, const void* pOutData, unsigned &nOutDataSize)
{
	if(!pInData || !nInDataSize || !pOutData || !nOutDataSize){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	int zRet;

	if(!m_Encoder.initialized){
		if((zRet = deflateInit(&m_Encoder.zs, Z_BEST_SPEED)) != Z_OK){
			OVNC_DEBUG_ERROR("deflateInit() failed %d", zRet);
			return vncError_3rdApplicationError;
		}
		m_Encoder.initialized = true;
	}

	m_Encoder.zs.next_in = (Bytef*)pInData;
	m_Encoder.zs.avail_in = nInDataSize;
	m_Encoder.zs.next_out = (Bytef*)pOutData;
	m_Encoder.zs.avail_out = nOutDataSize;

	if((zRet = deflate(&m_Encoder.zs, Z_SYNC_FLUSH)) != Z_OK){
		OVNC_DEBUG_ERROR("deflate() failed %d", zRet);
		return vncError_3rdApplicationError;
	}

	nOutDataSize -= m_Encoder.zs.avail_out;
	return vncError_Ok;
}

unsigned vncRfbEncodingZRLE::GetTileWidth()
{
	return OVNC_RFB_ZRLE_TILE_WIDTH;
}

unsigned vncRfbEncodingZRLE::GetTileHeight()
{
	return OVNC_RFB_ZRLE_TILE_HEIGHT;
}

vncError_t vncRfbEncodingZRLE::Encode(const vncObjectWrapper<vncRfbFrameBuffer*> pVNCFrameBuffer, const vncRfbRectangle_t* pOutRectangle, const vncRfbPixelFormat_t* pOutFormat, const void* pOutData, unsigned &nOutDataSize)
{
	// call base class function
	return vncRfbEncodingRLE::Encode(pVNCFrameBuffer,  pOutRectangle, pOutFormat, pOutData, nOutDataSize);
}

vncError_t vncRfbEncodingZRLE::Decode(vncObjectWrapper<vncVideoFrame*>pVNCVideoFrame, vncObjectWrapper<vncRfbFrameBuffer*> pVNCFrameBuffer)
{
	// call base class function
	return vncRfbEncodingRLE::Decode(pVNCVideoFrame, pVNCFrameBuffer);
}

vncError_t vncRfbEncodingZRLE::ResetState()
{
	if(m_Decoder.initialized){
		m_Decoder.initialized = false;
		inflateEnd(&m_Decoder.zs);
		memset(&m_Decoder.zs, 0, sizeof(m_Decoder.zs));
		m_Decoder.zs.data_type = Z_BINARY;
	}
	return vncError_Ok;
}