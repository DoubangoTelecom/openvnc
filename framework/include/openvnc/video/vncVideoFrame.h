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
#ifndef OVNC_VIDEO_FRAME_H
#define OVNC_VIDEO_FRAME_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncBuffer.h"
#include "openvnc/rfb/vncRfbTypes.h"

class vncVideoFrame : public vncObject
{
public:
	vncVideoFrame(const vncRfbPixelFormat_t* pFormat, const void* pData, unsigned nDataSize, const vncRfbRectangle_t *pRectangle);
	vncVideoFrame(const vncRfbPixelFormat_t* pFormat, vncObjectWrapper<vncBuffer*>pVNCData,  const vncRfbRectangle_t *pRectangle);
	virtual ~vncVideoFrame();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncVideoFrame"; }
	virtual OVNC_INLINE unsigned GetWidth(){ return m_Rectangle.width; }
	virtual OVNC_INLINE unsigned GetHeight(){ return m_Rectangle.height; }
	virtual OVNC_INLINE const vncRfbPixelFormat_t* GetPixelFormat(){ return &m_PixelFormat; }
	virtual OVNC_INLINE vncObjectWrapper<vncBuffer*> GetData(){ return m_pVNCData; }
	virtual OVNC_INLINE vncRfbEncoding_t GetEncoding(){ return m_Rectangle.encoding_type; }
	virtual OVNC_INLINE vncRfbRectangle_t* GetRectangle(){ return &m_Rectangle; }

private:
	void SetFormat(const vncRfbPixelFormat_t* pFormat, const vncRfbRectangle_t *pRectangle);

private:
	vncObjectWrapper<vncBuffer*>m_pVNCData;
	vncRfbPixelFormat_t m_PixelFormat;
	vncRfbRectangle_t m_Rectangle;
};

#endif /* OVNC_VIDEO_FRAME_H */
