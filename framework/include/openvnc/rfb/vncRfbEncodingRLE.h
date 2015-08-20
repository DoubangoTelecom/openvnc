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
#ifndef OVNC_RFB_ENCODING_RLE_H
#define OVNC_RFB_ENCODING_RLE_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncByteWriter.h"
#include "openvnc/rfb/vncRfbTypes.h"
#include "openvnc/rfb/vncRfbEncoding.h"
#include "openvnc/rfb/vncRfbPixelReader.h"

class vncRfbEncodingRLE : public vncRfbEncoding
{
protected:
	vncRfbEncodingRLE(vncRfbEncoding_t eEncoding);
public:
	virtual ~vncRfbEncodingRLE();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncRfbEncodingRLE"; }
	virtual vncError_t Encode(const vncObjectWrapper<vncRfbFrameBuffer*> pVNCFrameBuffer, const vncRfbRectangle_t* pOutRectangle, const vncRfbPixelFormat_t* pOutFormat, const void* pOutData, unsigned &nOutDataSize);
	virtual vncError_t Decode(vncObjectWrapper<vncVideoFrame*>pVNCVideoFrame, vncObjectWrapper<vncRfbFrameBuffer*> pVNCFrameBuffer);

protected:
	virtual vncError_t DecodeCompressed(vncObjectWrapper<vncVideoFrame*>pVNCVideoFrame, const void* pOutData, unsigned &nOutDataSize) = 0;
	virtual vncError_t EncodeRaw(const void* pInData, unsigned nInDataSize, const void* pOutData, unsigned &nOutDataSize) = 0;
	virtual unsigned GetTileWidth()=0;
	virtual unsigned GetTileHeight()=0;


private:
	struct{
		vncRfbPixel_t* pTileData;
		void* pOutData;
		unsigned nOutDataSize;
		vncObjectWrapper<vncRfbPixelReader*> pVNCPixelReader;
	} m_Decode;

	struct{
		vncRfbPixel_t* pTileData;
		void* pOutData;
		unsigned nOutDataSize;
		vncRfbPixel_t* pTilePixels;
		unsigned* pTilePixelsLengths;
		unsigned* pTilePixelsIndexes;
		vncObjectWrapper<vncRfbPixelReader*> pVNCPixelReader;
		vncObjectWrapper<vncByteWriter*> pVNCByteWriter;
	} m_Encode;
};


#endif /* OVNC_RFB_ENCODING_RLE_H */
