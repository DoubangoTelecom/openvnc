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
#include "openvnc/vncByteWriter.h"

vncByteWriter::vncByteWriter(const void* pData, unsigned nDataSize)
{
	SetData(pData, nDataSize);
}

vncByteWriter::~vncByteWriter()
{
}

vncError_t vncByteWriter::SetData(const void* pcData, unsigned nDataSize)
{
	m_pcData = pcData;
	m_nDataSize = nDataSize;
	m_nDataIndex = 0;

	return vncError_Ok;
}

vncError_t vncByteWriter::Write(uint32_t nVal, unsigned nBytesCount, int32_t nIndex, bool bMoveIndex)
{
	if(!m_pcData || !m_nDataSize || !nBytesCount || nBytesCount > 4){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}

	if(nIndex < 0){
		nIndex = m_nDataIndex;
	}

	if((nIndex + nBytesCount) <= m_nDataSize){
		uint8_t* pcData = &((uint8_t*)m_pcData)[nIndex];
		switch(nBytesCount){
			case 1:
				{
					pcData[0] = nVal & 0xFF;
					break;
				}
			case 2:
				{
					pcData[1] =  (nVal >> 8) & 0xFF;
					pcData[0] =  nVal & 0xFF;
					break;
				}
			case 3:
				{
					pcData[2] =  (nVal >> 16) & 0xFF;
					pcData[1] =  (nVal >> 8) & 0xFF;
					pcData[0] =  nVal & 0xFF;
					break;
				}
			case 4:
				{
					pcData[3] =  (nVal >> 24) & 0xFF;
					pcData[2] =  (nVal >> 16) & 0xFF;
					pcData[1] =  (nVal >> 8) & 0xFF;
					pcData[0] =  nVal & 0xFF;
					break;
				}
		}
	}
	else{
		OVNC_DEBUG_ERROR("BufferOverflow");
		return vncError_BufferOverflow;
	}

	if(bMoveIndex){
		m_nDataIndex = (nIndex + nBytesCount);
		m_nDataIndex = OVNC_MIN(m_nDataIndex, m_nDataSize);
	}
	return vncError_Ok;
}
