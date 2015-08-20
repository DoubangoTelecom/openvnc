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
#ifndef OVNC_VIDEO_GRABBER_H
#define OVNC_VIDEO_GRABBER_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncMutex.h"
#include "openvnc/video/vncVideoFrame.h"
#include "openvnc/rfb/vncRfbTypes.h"
#include "openvnc/rfb/vncRfbEncoding.h"

#include <map>

class OVNC_API vncScreenGrabber : public vncObject
{
protected:
	vncScreenGrabber(const void* pParent = NULL);

public:
	virtual ~vncScreenGrabber();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncScreenGrabber"; }
	virtual vncObjectWrapper<vncRfbEncoding*> vncScreenGrabber::RequestEncoding(vncRfbEncoding_t eEncoding);
	virtual bool IsValid() = 0;
	virtual const vncRfbPixelFormat_t* GetNativeFormat() = 0;
	virtual unsigned GetWidth() = 0;
	virtual unsigned GetHeight() = 0;
	virtual vncError_t Capture(const vncRfbPixelFormat_t* pOutFormat, const vncRfbRectangle_t* pOutRectangle, const void** pOutBuffer, unsigned &nOutBufferSize) = 0;

	static vncObjectWrapper<vncScreenGrabber*> New(const void* pParent=NULL);

	static vncError_t Initialize();

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable : 4251 )
#endif

private:
	static bool g_bInitialized;
	vncObjectWrapper<vncMutex*>m_pEncodingsMutex;
	std::map<vncRfbEncoding_t, vncObjectWrapper<vncRfbEncoding*> >m_aEncodings;

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
};

#endif /* OVNC_VIDEO_GRABBER_H */
