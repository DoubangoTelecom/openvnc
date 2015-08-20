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
#include "openvnc/video/vncVideoFrame.h"

vncVideoFrame::vncVideoFrame(const vncRfbPixelFormat_t* pFormat, const void* pData, unsigned nDataSize, const vncRfbRectangle_t *pRectangle)
:vncObject()
{
	m_pVNCData = vncBuffer::New(pData, nDataSize);
	SetFormat(pFormat, pRectangle);	
}

vncVideoFrame::vncVideoFrame(const vncRfbPixelFormat_t* pFormat, vncObjectWrapper<vncBuffer*>pVNCData, const vncRfbRectangle_t *pRectangle)
{
	m_pVNCData = pVNCData;
	SetFormat(pFormat, pRectangle);
}

void vncVideoFrame::SetFormat(const vncRfbPixelFormat_t* pFormat, const vncRfbRectangle_t *pRectangle)
{
	if(pRectangle){
		memcpy(&m_Rectangle, pRectangle, sizeof(vncRfbRectangle_t));
	}
	else{
		memset(&m_Rectangle, 0, sizeof(vncRfbRectangle_t));
	}
	if(pFormat){
		memcpy(&m_PixelFormat, pFormat, sizeof(vncRfbPixelFormat_t));
	}
	else{
		memset(&m_PixelFormat, 0, sizeof(vncRfbPixelFormat_t));
	}
}

vncVideoFrame::~vncVideoFrame()
{
	vncObjectSafeFree(m_pVNCData);
}
