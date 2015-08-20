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
#include "openvnc/vncBuffer.h"
#include "openvnc/vncUtils.h"

vncBuffer::vncBuffer(const void* pData, unsigned nDataSize)
{
	m_pData = NULL;
	m_nDataSize = 0;

	if(pData && nDataSize){
		if((m_pData = vncUtils::Malloc(nDataSize))){
			memcpy(m_pData, pData, nDataSize);
			m_nDataSize = nDataSize;
		}
	}
}

vncBuffer::~vncBuffer()
{
	OVNC_SAFE_FREE(m_pData);
}

vncError_t vncBuffer::CleanUp()
{
	if(m_pData){
		vncUtils::Free(&m_pData);
	}
	m_nDataSize = 0;
	return vncError_Ok;
}

vncError_t vncBuffer::Append(const void* pData, unsigned nSize)
{
	if(pData && nSize){
		unsigned nOldSize = m_nDataSize;
		unsigned nNewSize = nOldSize + nSize;
		
		if(nOldSize){
			m_pData = vncUtils::Realloc(m_pData, nNewSize);
		}
		else{
			m_pData = vncUtils::Calloc(nSize, sizeof(uint8_t));
		}

		if(m_pData){
			memcpy(((uint8_t*)m_pData) + nOldSize, pData, nSize);
			m_nDataSize = nNewSize;
			return vncError_Success;
		}
		else{
			m_nDataSize = 0;
			return vncError_ApplicationError;
		}
	}
	else{
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	
	return vncError_InvalidParameter;
}

vncError_t vncBuffer::Realloc(unsigned nSize)
{
	if(nSize == 0){
		return CleanUp();
	}

	if(m_nDataSize == 0){ // first time?
		m_pData = vncUtils::Calloc(nSize, sizeof(uint8_t));
	}
	else if(m_nDataSize != nSize){ // only realloc if different sizes
		m_pData = vncUtils::Realloc(m_pData, nSize);
	}

	m_nDataSize = nSize;
	return vncError_Ok;
}

vncError_t vncBuffer::Remove(unsigned nPosition, unsigned nSize)
{
	if(m_pData && m_nDataSize){
		if((nPosition == 0) && ((nPosition + nSize) >= m_nDataSize)){ /* Very common case. */
			return CleanUp();
		}
		else if((nPosition + nSize) < m_nDataSize){
			memcpy(((uint8_t*)m_pData) + nPosition, ((uint8_t*)m_pData) + nPosition + nSize, 
				m_nDataSize - (nPosition + nSize));
			return Realloc(m_nDataSize - nSize);
		}
	}
	return vncError_Ok;
}

vncError_t vncBuffer::Copy(unsigned nStart, const void* pData, unsigned nSize)
{
	vncError_t ret;
	if(!pData || !nSize){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	
	// realloc the buffer to match the overral size
	if((ret = Realloc((nStart + nSize))) != vncError_Ok){
		OVNC_DEBUG_ERROR("Failed to realloc the buffer");
		return ret;
	}

	memcpy(((uint8_t*)m_pData) + nStart, pData, nSize);
	return ret;
}

vncError_t vncBuffer::TakeOwnership(void** ppData, unsigned nSize)
{
	if(!ppData || !*ppData || !nSize){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}

	if(m_pData){
		vncUtils::Free(&m_pData);
	}
	m_pData = *ppData;
	m_nDataSize = nSize;
	*ppData = NULL;

	return vncError_Ok;
}

vncObjectWrapper<vncBuffer*> vncBuffer::New(const void* pData, unsigned nDataSize)
{
	return new vncBuffer(pData, nDataSize);
}