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
#include "openvnc/rfb/vncRfbPixelReader.h"
#include "openvnc/network/vncNetUtils.h"

static inline vncRfbPixel_t BuildPixel(const vncRfbPixelFormat_t* pPixelFormat, int32_t R, int32_t G, int32_t B, int32_t A = (int32_t)0xFF)
{
	return 
		(((R & pPixelFormat->red_max) << pPixelFormat->red_shift) |
		((G & pPixelFormat->green_max) << pPixelFormat->green_shift) |
		((B & pPixelFormat->blue_max) << pPixelFormat->blue_shift));
}

static inline int32_t GetColorFromPixel(int32_t pixel, int32_t shift, int32_t maxVal)
{
	return (((pixel >> shift) & maxVal) * 0xFF / maxVal);
}

vncRfbPixelReader::vncRfbPixelReader(const vncRfbPixelFormat_t* pInPixelFormat, const vncRfbPixelFormat_t* pOutPixelFormat, vncRfbEncoding_t eInEncodingType, vncRfbEncoding_t eOutEncodingType, const void* pcData, unsigned nDataSize)
:vncObject()
{
	Reset(pInPixelFormat, pOutPixelFormat, eInEncodingType, eOutEncodingType, pcData, nDataSize);
}

vncRfbPixelReader::~vncRfbPixelReader()
{
	m_pcData = NULL;
	m_nDataSize = 0;
}

vncError_t vncRfbPixelReader::Reset(const vncRfbPixelFormat_t* pInPixelFormat, const vncRfbPixelFormat_t* pOutPixelFormat, vncRfbEncoding_t eInEncodingType, vncRfbEncoding_t eOutEncodingType, const void* pcData, unsigned nDataSize)
{
	SetInPixelFormat(pInPixelFormat);
	SetOutPixelFormat(pOutPixelFormat);
	SetData(pcData, nDataSize, eInEncodingType);
	m_eOutEncodingType = eOutEncodingType;
	return vncError_Ok;
}

vncError_t vncRfbPixelReader::SetData(const void* pcData, unsigned nDataSize, vncRfbEncoding_t eEncodingType)
{
	m_pcData = pcData;
	m_nDataSize = nDataSize;
	m_nDataIndex = 0;
	m_eInEncodingType = eEncodingType;
	return vncError_Ok;
}

const void* vncRfbPixelReader::GetData(int32_t nIndex)
{
	if(nIndex < 0){
		nIndex = m_nDataIndex;
	}
	if(nIndex < (int32_t)m_nDataSize){
		return ((const uint8_t*)m_pcData + nIndex);
	}
	return NULL;
}

void vncRfbPixelReader::SetInPixelFormat(const vncRfbPixelFormat_t* pPixelFormat)
{
	if(pPixelFormat){
		memcpy(&m_InPixelFormat, pPixelFormat, sizeof(vncRfbPixelFormat_t));
	}
}

void vncRfbPixelReader::SetOutPixelFormat(const vncRfbPixelFormat_t* pPixelFormat){
	if(pPixelFormat){
		memcpy(&m_OutPixelFormat, pPixelFormat, sizeof(vncRfbPixelFormat_t));
	}
}

unsigned vncRfbPixelReader::GetInPixelSize()
{
	if((m_eInEncodingType == vncRfbEncoding_ZRLE) && m_InPixelFormat.true_colour_flag && m_InPixelFormat.bits_per_pixel == 32 && m_InPixelFormat.depth <= 24){
		return 3;//CPIXEL
	}
	return m_InPixelFormat.bits_per_pixel / 8;
}

unsigned vncRfbPixelReader::GetOutPixelSize()
{
	if((m_eOutEncodingType == vncRfbEncoding_ZRLE) && m_OutPixelFormat.true_colour_flag && m_OutPixelFormat.bits_per_pixel == 32 && m_OutPixelFormat.depth <= 24){
		return 3;//CPIXEL
	}
	return m_OutPixelFormat.bits_per_pixel / 8;
}

vncRfbPixel_t vncRfbPixelReader::ReadPixel(int32_t nIndex, bool bMoveIndex)
{
	if(!m_pcData || !m_nDataSize){
		OVNC_DEBUG_ERROR("Invalid data");
		return 0;
	}

	if(nIndex < 0){
		nIndex = m_nDataIndex;
	}
	unsigned nBytesCount = 0;
	vncRfbPixel_t nPixel = 0;
	uint8_t *_pcData = (uint8_t*)m_pcData;
	nBytesCount = GetInPixelSize();

	if((nIndex + nBytesCount) <= m_nDataSize){//yep "<=" is correct because of zero-based index
		
		unsigned long _pixel = nBytesCount > 2 ? OVNC_TO_UINT32(&_pcData[nIndex]) : 
									(nBytesCount > 1 ? OVNC_TO_UINT16(&_pcData[nIndex]) : _pcData[nIndex]);
		nPixel = BuildPixel(&m_OutPixelFormat,
			GetColorFromPixel(_pixel, m_InPixelFormat.red_shift, m_InPixelFormat.red_max),
			GetColorFromPixel(_pixel, m_InPixelFormat.green_shift, m_InPixelFormat.green_max),
			GetColorFromPixel(_pixel, m_InPixelFormat.blue_shift, m_InPixelFormat.blue_max)
		);
	}
	else{
		OVNC_DEBUG_ERROR("BufferOverflow");
		goto done;
	}

done:
	if(bMoveIndex){
		m_nDataIndex = (nIndex + nBytesCount);
		m_nDataIndex = OVNC_MIN(m_nDataIndex, m_nDataSize);
	}

	return nPixel;
}

uint8_t vncRfbPixelReader::ReadByte(int32_t nIndex, bool bMoveIndex)
{
	if(!m_pcData || !m_nDataSize){
		OVNC_DEBUG_ERROR("Invalid data");
		return 0;
	}

	if(nIndex < 0){
		nIndex = m_nDataIndex;
	}
	uint8_t byte = 0;
	if(nIndex < (int32_t)m_nDataSize){
		byte = ((uint8_t*)m_pcData)[nIndex];
	}
	else{
		OVNC_DEBUG_ERROR("BufferOverflow");
	}

	if(bMoveIndex){
		m_nDataIndex = ++nIndex;
		m_nDataIndex = OVNC_MIN(m_nDataIndex, m_nDataSize);
	}
	return byte;
}

vncError_t vncRfbPixelReader::SkipBytes(unsigned nCount, int32_t nIndex)
{
	if(nIndex < 0){
		nIndex = m_nDataIndex;
	}
	m_nDataIndex += nCount;
	m_nDataIndex = OVNC_MIN(m_nDataIndex, m_nDataSize);
	return vncError_Ok;
}

vncObjectWrapper<vncRfbPixelReader*> vncRfbPixelReader::New(const vncRfbPixelFormat_t* pInPixelFormat, const vncRfbPixelFormat_t* pOutPixelFormat, vncRfbEncoding_t eInEncodingType, vncRfbEncoding_t eOutEncodingType, const void* pcData, unsigned nDataSize)
{
	if(pInPixelFormat && pOutPixelFormat){
		return new vncRfbPixelReader(pInPixelFormat, pOutPixelFormat, eInEncodingType, eOutEncodingType, pcData, nDataSize);
	}
	return NULL;	
}
