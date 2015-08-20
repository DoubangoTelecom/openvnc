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
#include "openvnc/video/vncDisplayGDI.h"
#include "openvnc/vncDebug.h"
#include "openvnc/network/vncNetUtils.h"

vncDisplayGDI::vncDisplayGDI(unsigned nWidth, unsigned nHeight, void* pParent)
:vncDisplay(nWidth, nHeight, pParent)
{
	m_pVNCMutex = vncMutex::New(false);

	m_hWindow = reinterpret_cast<HWND>(pParent);
	m_bWindowOwner = (m_hWindow == NULL);
	if(m_bWindowOwner){
		m_hWindow = vncDisplayGDI::CreateFloatingWindow(m_nWidth, m_nHeight);
	}
#if 0
	else{
		SetWindowLongPtr(m_hWindow, GWL_WNDPROC, (LONG)vncDisplayGDI::WndProc);
	}
#endif

	if(m_hWindow){
		SetPropA(m_hWindow, "This", this);
		ShowWindow(m_hWindow, SW_SHOW);
	}

	m_nDrawPositionX = 0;
	m_nDrawPositionY = 0;

	vncRfbPixelFormat_t gdiPixelFormat;
	memset(&gdiPixelFormat, 0, sizeof(vncRfbPixelFormat_t));
	gdiPixelFormat.bits_per_pixel = 32;
	gdiPixelFormat.depth = 24;
	gdiPixelFormat.big_endian_flag = vncNetUtils::IsBigEndian() ? 1 : 0;
	gdiPixelFormat.true_colour_flag = 1;
	gdiPixelFormat.red_max = (1<<8) - 1;
	gdiPixelFormat.green_max = (1<<8) - 1;
	gdiPixelFormat.blue_max = (1<<8) - 1;
	gdiPixelFormat.blue_shift = 0;
	gdiPixelFormat.green_shift = 8;
	gdiPixelFormat.red_shift = 16;	

	m_pVNCFrameBuffer = vncRfbFrameBuffer::New(nWidth, nHeight, &gdiPixelFormat);

	memset(&m_BitmapInfo, 0, sizeof(m_BitmapInfo));
	m_BitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
	m_BitmapInfo.biWidth = m_nWidth;
	m_BitmapInfo.biHeight = m_nHeight;
	m_BitmapInfo.biPlanes = 1;
	m_BitmapInfo.biCompression = BI_RGB;
	m_BitmapInfo.biBitCount = 32;
}

vncDisplayGDI::~vncDisplayGDI()
{
	if(m_hWindow && m_bWindowOwner){
		vncDisplayGDI::DestroyFloatingWindow(m_hWindow);
	}
	vncObjectSafeFree(m_pVNCMutex);
	vncObjectSafeFree(m_pVNCFrameBuffer);
}

BOOL vncDisplayGDI::Initialize()
{
	static BOOL g_bInitialized = FALSE;
	if(!g_bInitialized){
		WNDCLASS wc		 = { 0 };
		wc.style         = /* CS_VREDRAW | CS_HREDRAW | */ CS_DBLCLKS;
		wc.lpfnWndProc   = WndProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = GetModuleHandle(NULL);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName  = 0;
		wc.lpszClassName = L"vncVideoDisplay";
		
		g_bInitialized = (RegisterClass(&wc) != NULL);
	}
	return TRUE;
}

void vncDisplayGDI::GdiDisplay(HDC hDC, HDC memDC)
{
	if(!m_hWindow){
		OVNC_DEBUG_ERROR("Not window associated");
		return;
	} 

	m_pVNCMutex->Lock();

	if(m_pVNCFrameBuffer){

		m_BitmapInfo.biBitCount = m_pVNCFrameBuffer->GetPixelFormat()->bits_per_pixel;
		m_BitmapInfo.biWidth = m_pVNCFrameBuffer->GetWidth();
		m_BitmapInfo.biHeight = m_pVNCFrameBuffer->GetHeight();
		m_BitmapInfo.biSizeImage = m_pVNCFrameBuffer->GetBufferSize();

		SetStretchBltMode(memDC ? memDC : hDC, HALFTONE);
		
		// Draw	
		StretchDIBits(
			memDC ? memDC : hDC,
			m_nDrawPositionX, m_nDrawPositionY, m_BitmapInfo.biWidth, m_BitmapInfo.biHeight,
			0, m_BitmapInfo.biHeight, m_BitmapInfo.biWidth, -m_BitmapInfo.biHeight, // Vertical flip
			m_pVNCFrameBuffer->GetBuffer(),
			reinterpret_cast<BITMAPINFO*>(&m_BitmapInfo),
			DIB_RGB_COLORS,
			SRCCOPY);
		// Transfer
		if(memDC){
			BitBlt(hDC, 0, 0, m_nWidth, m_nHeight, memDC, 0, 0, SRCCOPY);
		}
	}

	m_pVNCMutex->UnLock();
}

HWND vncDisplayGDI::CreateFloatingWindow(unsigned nWidth, unsigned nHeight)
{
	HINSTANCE hInst = GetModuleHandle(NULL);
    return CreateWindowA("vncVideoDisplay", "Should be ServerName",//FIXME
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT/*nWidth, nHeight*/, NULL, 0, hInst, NULL);//FIXME
}

void vncDisplayGDI::DestroyFloatingWindow(HWND &hWnd)
{
	if(DestroyWindow(hWnd)){
		hWnd = NULL;
	}
}

LRESULT CALLBACK vncDisplayGDI::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_PAINT:
			{
				vncDisplayGDI* This = (vncDisplayGDI*)GetPropA(hWnd, "This");
				if(This){
					PAINTSTRUCT ps;
					HDC hDC = BeginPaint(hWnd, &ps);

#if 1 // Double Buffering
					HDC memDC = ::CreateCompatibleDC(hDC);
					HBITMAP hBmp = ::CreateCompatibleBitmap(hDC, This->m_nWidth, This->m_nHeight);
					HGDIOBJ hTempBmp = ::SelectObject(memDC, hBmp);

					RECT rect;
					::GetClientRect(hWnd, &rect);
					::FillRect(memDC, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
					
					This->GdiDisplay(hDC, memDC);
					
					::SelectObject(memDC, hTempBmp);
					::DeleteObject(hBmp);
					::DeleteDC(memDC);
#else
					This->GdiDisplay(hDC, NULL);
#endif
					EndPaint(hWnd, &ps);
				}
				break;
			}

		case WM_SIZE:
		{
			vncDisplayGDI* This = (vncDisplayGDI*)GetPropA(hWnd, "This");
			if(This){
				RECT rect = {0};
				GetWindowRect(hWnd, &rect);

				This->m_nWidth = (rect.right - rect.left);
				This->m_nHeight = (rect.bottom - rect.top);
				if(This->m_pVNCFrameBuffer){
					This->m_nDrawPositionX = (This->m_nWidth > This->m_pVNCFrameBuffer->GetWidth()) ? (This->m_nWidth - This->m_pVNCFrameBuffer->GetWidth()) / 2 : 0;
					This->m_nDrawPositionY = (This->m_nHeight > This->m_pVNCFrameBuffer->GetHeight()) ? (This->m_nHeight - This->m_pVNCFrameBuffer->GetHeight()) / 2 : 0;
				}
			}
			break;
		}

		case WM_DESTROY:
			{
				vncDisplayGDI* This = (vncDisplayGDI*)GetPropA(hWnd, "This");
				if(This){
					SetWindowLongPtr(This->m_hWindow, GWL_WNDPROC, (LONG)NULL);
					This->m_hWindow = NULL;
				}
				break;
			}
		
		case WM_ERASEBKGND:
			{
				return TRUE;
			}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

vncError_t vncDisplayGDI::Display(vncObjectWrapper<vncVideoFrame*>pVNCVideoFrame)
{
	if(m_hWindow && pVNCVideoFrame && pVNCVideoFrame->GetData()){
		m_pVNCMutex->Lock();
		
		// We must decode it right now ==> Because of StateFull encoders
		vncObjectWrapper<vncRfbEncoding*>pVNCEncoding;
		if(pVNCVideoFrame->GetEncoding() == vncRfbEncoding_Raw){
			OVNC_DEBUG_ERROR("Not Implemented");
			return vncError_NotImplemented;
		}
		else if((pVNCEncoding = RequestEncoding(pVNCVideoFrame->GetEncoding()))){
			vncError_t ret = pVNCEncoding->Decode(pVNCVideoFrame, m_pVNCFrameBuffer);
			if(ret == vncError_Ok){
#if 0
				RECT rec;
				rec.left = pVNCVideoFrame->GetRectangle()->x_position;
				rec.top = pVNCVideoFrame->GetRectangle()->y_position;
				rec.bottom = rec.top + pVNCVideoFrame->GetRectangle()->height;
				rec.right = rec.left + pVNCVideoFrame->GetRectangle()->width;
				InvalidateRect(m_hWindow, &rec, FALSE);
#else
				InvalidateRect(m_hWindow, NULL, FALSE);
#endif
			}
		}
		
		m_pVNCMutex->UnLock();
		
		// "UpdateWindow" is synchronous -> Must be called after UnLock()
		if(m_bWindowOwner){
			UpdateWindow(m_hWindow);
		}
		return vncError_Ok;
	}
	else{
		OVNC_DEBUG_ERROR("Not window associated");
		return vncError_InvalidState;
	}
}

vncError_t vncDisplayGDI::AdjustMousePointerToFit(uint16_t &nXPosition, uint16_t &nYPosition)
{
	nXPosition -= m_nDrawPositionX;
	nYPosition -= m_nDrawPositionY;
	return vncError_Ok;
}

bool vncDisplayGDI::IsValid()
{
	return vncDisplay::IsValid() && (m_pVNCFrameBuffer);
}