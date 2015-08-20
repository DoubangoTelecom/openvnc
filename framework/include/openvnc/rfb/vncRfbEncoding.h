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
#ifndef OVNC_RFB_ENCODING_H
#define OVNC_RFB_ENCODING_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/rfb/vncRfbTypes.h"
#include "openvnc/video/vncVideoFrame.h"
#include "openvnc/rfb/vncRfbFrameBuffer.h"

class vncRfbEncoding : public vncObject
{
protected:
	vncRfbEncoding(vncRfbEncoding_t eEncoding);
public:
	virtual ~vncRfbEncoding();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncRfbEncoding"; }
	virtual OVNC_INLINE vncRfbEncoding_t GetEncoding() { return m_eEncoding; }
	virtual vncError_t Encode(const vncObjectWrapper<vncRfbFrameBuffer*> pVNCFrameBuffer, const vncRfbRectangle_t* pOutRectangle, const vncRfbPixelFormat_t* pOutFormat, const void* pOutData, unsigned &nOutDataSize) = 0;
	virtual vncError_t Decode(vncObjectWrapper<vncVideoFrame*>pVNCVideoFrame, vncObjectWrapper<vncRfbFrameBuffer*> pVNCFrameBuffer)=0;
	virtual vncError_t ResetState()=0;

	static vncObjectWrapper<vncRfbEncoding*> New(vncRfbEncoding_t eEncoding);

private:
	vncRfbEncoding_t m_eEncoding;
};


#endif /* OVNC_RFB_ENCODING_H */
