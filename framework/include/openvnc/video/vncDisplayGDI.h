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
#ifndef OVNC_VIDEO_DISPLAY_GDI_H
#define OVNC_VIDEO_DISPLAY_GDI_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncMutex.h"
#include "openvnc/vncThread.h"
#include "openvnc/video/vncDisplay.h"
#include "openvnc/rfb/vncRfbFrameBuffer.h"

class vncDisplayGDI : public vncDisplay
{
public:
	vncDisplayGDI(unsigned nWidth, unsigned nHeight, void* pParent=NULL);
	virtual ~vncDisplayGDI();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncDisplayGDI"; }

	// Override From vncDisplay
	virtual vncError_t Display(vncObjectWrapper<vncVideoFrame*>pVNCVideoFrame);
	virtual vncError_t AdjustMousePointerToFit(uint16_t &nXPosition, uint16_t &nYPosition);
	virtual bool IsValid();
	
	static BOOL Initialize();
	

private:
	void GdiDisplay(HDC hDC, HDC memDC);
	static HWND CreateFloatingWindow(unsigned nWidth, unsigned nHeight);
	static void DestroyFloatingWindow(HWND &hWnd);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hWindow;
	bool m_bWindowOwner;
	BITMAPINFOHEADER m_BitmapInfo;
	unsigned m_nDrawPositionX;
	unsigned m_nDrawPositionY;
	vncObjectWrapper<vncRfbFrameBuffer*>m_pVNCFrameBuffer;
	vncObjectWrapper<vncMutex*>m_pVNCMutex;
};

#endif /* OVNC_VIDEO_DISPLAY_GDI_H */ 
