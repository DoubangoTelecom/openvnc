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
#include "openvnc/video/vncScreenGrabberGDI.h"
#include "openvnc/network/vncNetUtils.h"
#include "openvnc/vncUtils.h"
#include "openvnc/rfb/vncRfbEncoding.h"// FIXME

#include <Gdiplus.h>

// http://support.microsoft.com/kb/230492

ULONG_PTR vncScreenGrabberGDI::g_pGdiplusToken = NULL;
bool vncScreenGrabberGDI::g_bInitialized = false;

static BOOL GetOptimalDIBFormat(HDC hdc, BITMAPINFOHEADER *pbi);
static BOOL GetRGBBitsPerPixel(HDC hdc, PINT pRed, PINT pGreen, PINT pBlue);

vncScreenGrabberGDI::vncScreenGrabberGDI(const void* pParent)
:vncScreenGrabber(pParent)
{
	m_bValid = false;
	m_hBitmap = NULL;
	m_hDC = NULL;
	m_pOutBuffer = NULL;
	m_nOutBufferSize = 0;
	m_pParent = reinterpret_cast<HWND>((void*)pParent);

	if(!m_pParent){
		m_pParent = GetDesktopWindow();
	}
	
	RECT rect;
	::GetClientRect(m_pParent, &rect);
#if 0
	m_nWidth = ::GetSystemMetrics(SM_CXSCREEN);
	m_nHeight = ::GetSystemMetrics(SM_CYSCREEN);
#else
	m_nWidth = (rect.right - rect.left);
	m_nHeight = (rect.bottom - rect.top);

	m_nWidth /= 2;
	m_nHeight /= 2;
#endif

	m_hDC = CreateCompatibleDC(GetDC(m_pParent));
	if(!m_hDC){
		vncUtils_SystemPrintLastError("CreateCompatibleDC failed");
		return;
	}
	SetStretchBltMode(m_hDC, HALFTONE);
	
	vncRfbPixelFormat_t pixelFormat;
	memset(&pixelFormat, 0, sizeof(vncRfbPixelFormat_t));
	pixelFormat.big_endian_flag = vncNetUtils::IsBigEndian() ? 1 : 0;
	pixelFormat.true_colour_flag = 1;

	INT Red, Green, Blue;
	if(false && GetOptimalDIBFormat(m_hDC, &m_BitmapInfo) && GetRGBBitsPerPixel(m_hDC, &Red, &Green, &Blue)){
		pixelFormat.bits_per_pixel = (Red + Green + Blue);
		pixelFormat.depth = (Red + Green + Blue);
		pixelFormat.red_max = (1<<Red) - 1;
		pixelFormat.green_max = (1<<Green) - 1;
		pixelFormat.blue_max = (1<<Blue) - 1;
		pixelFormat.blue_shift = 0;
		pixelFormat.green_shift = Blue;
		pixelFormat.red_shift = (Green + Blue);
	}
	else{		
		pixelFormat.bits_per_pixel = 32;
		pixelFormat.depth = 24;
		pixelFormat.red_max = (1<<8) - 1;
		pixelFormat.green_max = (1<<8) - 1;
		pixelFormat.blue_max = (1<<8) - 1;
		pixelFormat.blue_shift = 0;
		pixelFormat.green_shift = 8;
		pixelFormat.red_shift = 16;

		memset(&m_BitmapInfo, 0, sizeof(m_BitmapInfo));
		m_BitmapInfo.biSize = sizeof(BITMAPINFOHEADER);
		m_BitmapInfo.biWidth = m_nWidth;
		m_BitmapInfo.biHeight = m_nHeight;
		m_BitmapInfo.biPlanes = 1;
		m_BitmapInfo.biCompression = BI_RGB;
		m_BitmapInfo.biBitCount = pixelFormat.bits_per_pixel;
		m_BitmapInfo.biSizeImage = (m_nWidth * m_nHeight) * pixelFormat.bits_per_pixel/8;
	}

	m_hBitmap = CreateCompatibleBitmap(GetDC(m_pParent), m_nWidth, m_nHeight);
	if(!m_hBitmap){
		vncUtils_SystemPrintLastError("CreateDIBSection(width=%u, height=%u) failed", m_nWidth, m_nHeight);
		return;
	}
	SelectObject(m_hDC, m_hBitmap);

	m_pVNCFrameBuffer = vncRfbFrameBuffer::New(m_nWidth, m_nHeight, &pixelFormat);

	m_pVNCMutex = vncMutex::New(false);

	m_bValid = (m_nWidth > 0 && m_nHeight > 0 && m_pVNCMutex && m_hDC && m_hBitmap && m_pVNCFrameBuffer);
}

vncScreenGrabberGDI::~vncScreenGrabberGDI()
{
	if(m_hDC){
		DeleteDC(m_hDC);
	}
	if(m_hBitmap){
		DeleteObject(m_hBitmap);
	}
	vncObjectSafeFree(m_pVNCFrameBuffer);
	vncObjectSafeFree(m_pVNCMutex);
	OVNC_SAFE_FREE(m_pOutBuffer);
}

bool vncScreenGrabberGDI::IsValid()
{
	return m_bValid;
}

const vncRfbPixelFormat_t* vncScreenGrabberGDI::GetNativeFormat()
{
	return m_pVNCFrameBuffer->GetPixelFormat();
}

unsigned vncScreenGrabberGDI::GetWidth()
{
	return m_nWidth;
}

unsigned vncScreenGrabberGDI::GetHeight()
{
	return m_nHeight;
}

vncError_t vncScreenGrabberGDI::Capture(const vncRfbPixelFormat_t* pOutFormat, const vncRfbRectangle_t* pOutRectangle, const void** pOutBuffer, unsigned &nOutBufferSize)
{
	nOutBufferSize = 0;
	if(!pOutFormat || !pOutRectangle || !pOutBuffer || *pOutBuffer){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}

	*pOutBuffer = NULL;
	vncError_t ret = vncError_Ok;
	vncObjectWrapper<vncRfbEncoding*> pVNCEncoding = RequestEncoding(pOutRectangle->encoding_type);

	if(!pVNCEncoding && pOutRectangle->encoding_type != vncRfbEncoding_Raw){//FIXME
		OVNC_DEBUG_ERROR("%d encoding not supported", pOutRectangle->encoding_type);
		return vncError_ApplicationError;
	}

	m_pVNCMutex->Lock();

	// Flip and transfer
	if(!StretchBlt(m_hDC, 0, 0, pOutRectangle->width, pOutRectangle->height,
		GetDC(m_pParent), 0, m_BitmapInfo.biHeight, m_BitmapInfo.biWidth, -m_BitmapInfo.biHeight, 
		SRCCOPY))
	{
		vncUtils_SystemPrintLastError("StretchBlt failed");
		ret = vncError_SystemError;
		if(vncUtils::SystemGetErrno() != ERROR_NOT_ENOUGH_MEMORY){
			goto bail;
		}
	}
	
	int win32Ret = GetDIBits(m_hDC, m_hBitmap, 0,
        m_nHeight,
		(LPVOID)m_pVNCFrameBuffer->GetBuffer(),//FIXME: GetBuffer(index)
        (BITMAPINFO *)&m_BitmapInfo,
		DIB_RGB_COLORS);

		(win32Ret);

		// alloc out buffer
		if(m_nOutBufferSize < m_pVNCFrameBuffer->GetBufferSize()){
			if(!(m_pOutBuffer = vncUtils::Realloc(m_pOutBuffer, m_pVNCFrameBuffer->GetBufferSize()))){
				OVNC_DEBUG_ERROR("Failed to allocate buffer");
				return vncError_MemoryAllocFailed;
			}
			m_nOutBufferSize = m_pVNCFrameBuffer->GetBufferSize();
		}

		if(pOutRectangle->encoding_type == vncRfbEncoding_Raw){//FIXME: create encoder		 
			*pOutBuffer = m_pVNCFrameBuffer->GetBuffer();
			nOutBufferSize = m_pVNCFrameBuffer->GetBufferSize();
			ret = vncError_Ok;
		}
		else if(pOutRectangle->encoding_type == vncRfbEncoding_ZRLE){ // ZRLE
			unsigned nEncodedDataSize = m_nOutBufferSize;
			ret = pVNCEncoding->Encode(m_pVNCFrameBuffer, pOutRectangle, pOutFormat, m_pOutBuffer, nEncodedDataSize);
			if(ret != vncError_Ok){
				OVNC_DEBUG_ERROR("Encoding failed");
				ret = vncError_ApplicationError;
				goto bail;
			}
		 
			*pOutBuffer = m_pOutBuffer;
			nOutBufferSize = nEncodedDataSize;
			ret = vncError_Ok;
		}

bail:
	m_pVNCMutex->UnLock();
	return ret;
}

BOOL vncScreenGrabberGDI::Initialize()
{
	if(vncScreenGrabberGDI::g_bInitialized){
		return TRUE;
	}

	using namespace Gdiplus;
	// Initialize GDI+
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&vncScreenGrabberGDI::g_pGdiplusToken, &gdiplusStartupInput, NULL);
	
	vncScreenGrabberGDI::g_bInitialized = TRUE;
	return TRUE;
}

BOOL vncScreenGrabberGDI::DeInitialize()
{
	if(!vncScreenGrabberGDI::g_bInitialized){
		return TRUE;
	}

	using namespace Gdiplus;
	// Uninitialize GDI+
	GdiplusShutdown(vncScreenGrabberGDI::g_pGdiplusToken);

	vncScreenGrabberGDI::g_bInitialized = FALSE;
	return TRUE;
}

/**********************************************************

GetOptimalDIBFormat

Purpose:
Retrieves the optimal DIB format for a display 
device. The optimal DIB format is the format that 
exactly matches the format of the target device. 
Obtaining this is very important when dealing with 
16bpp modes because you need to know what bitfields 
value to use (555 or 565 for example).

You normally use this function to get the best
format to pass to CreateDIBSection() in order to
maximize blt'ing performance.

Input:
hdc - Device to get the optimal format for.
pbi - Pointer to a BITMAPINFO + color table
(room for 256 colors is assumed).

Output:
pbi - Contains the optimal DIB format. In the 
<= 8bpp case, the color table will contain the 
system palette. In the >=16bpp case, the "color 
table" will contain the correct bit fields (see 
BI_BITFIELDS in the Platform SDK documentation 
for more information).

Notes:
If you are going to use this function on a 8bpp device
you should make sure the color table contains a identity
palette for optimal blt'ing.

**********************************************************/ 
static BOOL GetOptimalDIBFormat(HDC hdc, BITMAPINFOHEADER *pbi)
{
	HBITMAP hbm;
	BOOL bRet = TRUE;

	// Create a memory bitmap that is compatible with the
	// format of the target device.
	hbm = CreateCompatibleBitmap(hdc, 1, 1);
	if (!hbm)
		return FALSE;

	// Initialize the header.
	ZeroMemory(pbi, sizeof(BITMAPINFOHEADER));
	pbi->biSize = sizeof(BITMAPINFOHEADER);

	// First call to GetDIBits will fill in the optimal biBitCount.
	bRet = GetDIBits(hdc, hbm, 0, 1, NULL, (BITMAPINFO*)pbi, DIB_RGB_COLORS);

	// Second call to GetDIBits will get the optimal color table, o
	// or the optimal bitfields values.
	if (bRet)
		bRet = GetDIBits(hdc, hbm, 0, 1, NULL, (BITMAPINFO*)pbi, DIB_RGB_COLORS);

	// Clean up.
	DeleteObject(hbm);

	return bRet;
}

// Counts the number of set bits in a DWORD.
static BYTE CountBits(DWORD dw)
{
	int iBits = 0;

	while (dw) {
		iBits += (dw & 1);
		dw >>= 1;
	}

	return iBits;
}

/**********************************************************

GetRGBBitsPerPixel

Purpose:
Retrieves the number of bits of color resolution for each 
color channel of a specified.

Input:
hdc - Device to get the color information for.

Output:
pRed   - Number of distinct red levels the device can display.
pGreen - Number of distinct green levels the device can display.
pBlue  - Number of distinct blue levels the device can display.

Notes:
This function does not return any meaningful information for
palette-based devices.

**********************************************************/ 
static BOOL GetRGBBitsPerPixel(HDC hdc, PINT pRed, PINT pGreen, PINT pBlue)
{
	BITMAPINFOHEADER *pbi;
	LPDWORD lpdw;
	BOOL bRet = TRUE;

	// If the target device is palette-based, then bail because there is no
	// meaningful way to determine separate RGB bits per pixel.
	if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) 
		return FALSE;  

	// Shortcut for handling 24bpp cases.
	if (GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL) == 24) {
		*pRed = *pGreen = *pBlue = 8;
		return TRUE;
	}

	// Allocate room for a header and a color table.
	pbi = (BITMAPINFOHEADER *)GlobalAlloc(GPTR, sizeof(BITMAPINFOHEADER) + 
		sizeof(RGBQUAD)*256);
	if (!pbi)
		return FALSE;

	// Retrieve a description of the device surface.
	if (GetOptimalDIBFormat(hdc, pbi)) {
		// Get a pointer to the bitfields.
		lpdw = (LPDWORD)((LPBYTE)pbi + sizeof(BITMAPINFOHEADER));

		*pRed   = CountBits(lpdw[0]);
		*pGreen = CountBits(lpdw[1]);
		*pBlue  = CountBits(lpdw[2]);
	} else
		bRet = FALSE;

	// Clean up.
	GlobalFree(pbi);

	return bRet;
}
