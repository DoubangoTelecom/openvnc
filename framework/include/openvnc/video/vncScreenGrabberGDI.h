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
#ifndef OVNC_VIDEO_GRABBER_GDI_H
#define OVNC_VIDEO_GRABBER_GDI_H

#include "vncConfig.h"
#include "openvnc/video/vncScreenGrabber.h"
#include "openvnc/vncMutex.h"
#include "openvnc/rfb/vncRfbFrameBuffer.h"

class vncScreenGrabberGDI : public vncScreenGrabber
{
public:
	vncScreenGrabberGDI(const void* pParent = NULL);
	virtual ~vncScreenGrabberGDI();
	
	// Override from "vncScreenGrabber"
	virtual bool IsValid();
	virtual const vncRfbPixelFormat_t* GetNativeFormat();
	virtual unsigned GetWidth();
	virtual unsigned GetHeight();
	virtual vncError_t Capture(const vncRfbPixelFormat_t* pOutFormat, const vncRfbRectangle_t* pOutRectangle, const void** pOutBuffer, unsigned &nOutBufferSize);

	static BOOL Initialize();
	static BOOL DeInitialize();

private:
	static ULONG_PTR g_pGdiplusToken;
	static bool g_bInitialized;

	bool m_bValid;
	HWND m_pParent;
	vncObjectWrapper<vncRfbFrameBuffer*> m_pVNCFrameBuffer;
	unsigned m_nWidth;
	unsigned m_nHeight;
	vncObjectWrapper<vncMutex*> m_pVNCMutex;
	HDC m_hDC;
	HBITMAP m_hBitmap;
	BITMAPINFOHEADER m_BitmapInfo;
	void* m_pOutBuffer;
	unsigned m_nOutBufferSize;
};

#endif /* OVNC_VIDEO_GRABBER_GDI_H */
