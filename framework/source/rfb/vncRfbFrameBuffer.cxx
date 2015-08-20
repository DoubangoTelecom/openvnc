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
#include "openvnc/rfb/vncRfbFrameBuffer.h"
#include "openvnc/vncUtils.h"
#include "openvnc/network/vncNetUtils.h"

vncRfbFrameBuffer::vncRfbFrameBuffer(uint16_t nWidth, uint16_t nHeight, const vncRfbPixelFormat_t* pPixelFormat)
:vncObject()
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_pBuffer = NULL;
	m_nBufferSize = 0;

	if(pPixelFormat){
		memcpy(&m_PixelFormat, pPixelFormat, sizeof(vncRfbPixelFormat_t));
		m_nBufferSize = (m_nWidth * m_nHeight * (pPixelFormat->bits_per_pixel / 8));
		if(!(m_pBuffer = vncUtils::Calloc(m_nBufferSize, 1))){
			m_nBufferSize = 0;
		}
	}
	m_pVNCByteWriter = new vncByteWriter(m_pBuffer, m_nBufferSize);
}

vncRfbFrameBuffer::~vncRfbFrameBuffer()
{
	OVNC_SAFE_FREE(m_pBuffer);
	
	vncObjectSafeFree(m_pVNCByteWriter);
}

vncError_t vncRfbFrameBuffer::WritePixels(const vncRfbPixelFormat_t* pPixelFormat, const vncRfbPixel_t* pPixels, unsigned nPixelsSize, const vncRfbRectangle_t *pDstRectangle)
{
	if(!pPixelFormat || !pPixels || !nPixelsSize || !pDstRectangle){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	
	if(!m_pVNCByteWriter){
		OVNC_DEBUG_ERROR("Not buffer writer");
		return vncError_ApplicationError;
	}

	m_pVNCByteWriter->SetData(m_pBuffer, m_nBufferSize);
	int32_t Bpp = (m_PixelFormat.bits_per_pixel / 8);
	int32_t*pBuffer = (int32_t*)m_pBuffer;

	unsigned nIndex = pDstRectangle->y_position * m_nWidth + pDstRectangle->x_position;
	const vncRfbPixel_t* pPixelsEnd = (pPixels + nPixelsSize);
	for (unsigned y = 0; y < pDstRectangle->height; ++y) {
		for (unsigned x = 0; x < pDstRectangle->width; ++x, ++nIndex) {
			if(pPixels >= pPixelsEnd){
				OVNC_DEBUG_ERROR("BufferOverflow");
				return vncError_BufferOverflow;
			}
			m_pVNCByteWriter->Write(*pPixels++, Bpp, nIndex*Bpp);
		}
		nIndex += (m_nWidth - pDstRectangle->width);//Tile: nextLine
	}
	return vncError_Ok;
}

vncObjectWrapper<vncRfbFrameBuffer*> vncRfbFrameBuffer::New(uint16_t nWidth, uint16_t nHeight, const vncRfbPixelFormat_t* pPixelFormat)
{
	vncObjectWrapper<vncRfbFrameBuffer*>pVNCFrameBuffer = new vncRfbFrameBuffer(nWidth, nHeight, pPixelFormat);
	if(pVNCFrameBuffer && !pVNCFrameBuffer->GetBuffer()){
		vncObjectSafeFree(pVNCFrameBuffer);
	}
	return pVNCFrameBuffer;
}
