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
#ifndef OVNC_RFB_PIXEL_READER_H
#define OVNC_RFB_PIXEL_READER_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/rfb/vncRfbTypes.h"

class vncRfbPixelReader : public vncObject
{
protected:
	vncRfbPixelReader(const vncRfbPixelFormat_t* pInPixelFormat, const vncRfbPixelFormat_t* pOutPixelFormat, vncRfbEncoding_t eInEncodingType, vncRfbEncoding_t eOutEncodingType, const void* pcData=NULL, unsigned nDataSize=0);
public:
	virtual ~vncRfbPixelReader();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncRfbPixelReader"; }
	virtual vncError_t Reset(const vncRfbPixelFormat_t* pInPixelFormat, const vncRfbPixelFormat_t* pOutPixelFormat, vncRfbEncoding_t eInEncodingType, vncRfbEncoding_t eOutEncodingType, const void* pcData=NULL, unsigned nDataSize=0);
	virtual vncError_t SetData(const void* pcData, unsigned nDataSize, vncRfbEncoding_t eEncodingType);
	virtual const void* GetData(int32_t nIndex = -1);
	virtual vncRfbPixel_t ReadPixel(int32_t nIndex = -1, bool bMoveIndex = true);
	virtual uint8_t ReadByte(int32_t nIndex = -1, bool bMoveIndex = true);
	virtual vncError_t SkipBytes(unsigned nCount, int32_t nIndex = -1);
	virtual OVNC_INLINE unsigned GetRemainingBytes()
	{
		return (m_nDataSize - m_nDataIndex);
	}
	virtual OVNC_INLINE void SetInPixelFormat(const vncRfbPixelFormat_t* pPixelFormat);
	virtual OVNC_INLINE void SetOutPixelFormat(const vncRfbPixelFormat_t* pPixelFormat);
	virtual OVNC_INLINE unsigned GetInPixelSize();
	virtual OVNC_INLINE unsigned GetOutPixelSize();
	virtual OVNC_INLINE const vncRfbPixelFormat_t* GetOutPixelFormat(){ return &m_OutPixelFormat; }
	virtual OVNC_INLINE const vncRfbPixelFormat_t* GetInPixelFormat(){ return &m_InPixelFormat; }

	static vncObjectWrapper<vncRfbPixelReader*>New(const vncRfbPixelFormat_t* pInPixelFormat, const vncRfbPixelFormat_t* pOutPixelFormat, vncRfbEncoding_t eInEncodingType, vncRfbEncoding_t eOutEncodingType, const void* pcData=NULL, unsigned nDataSize=0);

private:
	vncRfbEncoding_t m_eInEncodingType;
	vncRfbEncoding_t m_eOutEncodingType;
	vncRfbPixelFormat_t m_InPixelFormat;
	vncRfbPixelFormat_t m_OutPixelFormat;

protected:
	const void* m_pcData;
	unsigned m_nDataSize;
	unsigned m_nDataIndex;
};

#endif /* OVNC_RFB_PIXEL_READER_H */
