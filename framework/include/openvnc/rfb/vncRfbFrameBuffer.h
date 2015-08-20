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
#ifndef OVNC_RFB_FRAMEBUFFER_H
#define OVNC_RFB_FRAMEBUFFER_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/rfb/vncRfbTypes.h"
#include "openvnc/vncByteWriter.h"
#include "openvnc/rfb/vncRfbPixelReader.h"

class vncRfbFrameBuffer : public vncObject
{
protected:
	vncRfbFrameBuffer(uint16_t nWidth, uint16_t nHeight, const vncRfbPixelFormat_t* pPixelFormat);
public:
	virtual ~vncRfbFrameBuffer();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncRfbFrameBuffer"; }
	virtual OVNC_INLINE uint16_t GetWidth(){ return m_nWidth; }
	virtual OVNC_INLINE uint16_t GetHeight(){ return m_nHeight; }
	virtual OVNC_INLINE const vncRfbPixelFormat_t* GetPixelFormat(){ return &m_PixelFormat; }
	virtual OVNC_INLINE const void* GetBuffer(){ return m_pBuffer; }
	virtual OVNC_INLINE unsigned GetBufferSize(){ return m_nBufferSize; }
	virtual vncError_t WritePixels(const vncRfbPixelFormat_t* pPixelFormat, const vncRfbPixel_t* pPixels, unsigned nPixelsSize, const vncRfbRectangle_t *pDstRectangle);

	static vncObjectWrapper<vncRfbFrameBuffer*> New(uint16_t nWidth, uint16_t nHeight, const vncRfbPixelFormat_t* pPixelFormat);

private:
	uint16_t m_nWidth;
	uint16_t m_nHeight;
	vncRfbPixelFormat_t m_PixelFormat;
	void* m_pBuffer;
	unsigned m_nBufferSize;
	vncObjectWrapper<vncByteWriter*>m_pVNCByteWriter;
};

#endif /* OVNC_RFB_FRAMEBUFFER_H */
