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
#include "openvnc/rfb/vncRfbMsg.h"
#include "openvnc/network/vncNetUtils.h"
#include "openvnc/rfb/vncRfbUtils.h"

//
//	vncMsg
//

vncMsg::vncMsg(vncRfbMsgType_t eType)
:vncObject()
{
	m_eType = eType;
}

vncMsg::~vncMsg()
{

}

vncObjectWrapper<vncMsg*> vncMsg::ParseServerMsg(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	if(!pData){
		OVNC_DEBUG_ERROR("Invalid parameter");
		eResult = vncError_InvalidParameter;
		return NULL;
	}
	if(nDataSize < 1){
		eResult = vncError_TooShort;
		return NULL;
	}
	
	const uint8_t* _pData = (uint8_t*)pData;
	vncObjectWrapper<vncMsg*> pVNCMsg;

	switch(_pData[0])
	{
		case vncRfbServerMsg_FramebufferUpdate:
			{
				vncObjectWrapper<vncMsgFramebufferUpdate*>_pVNCMsg = vncMsgFramebufferUpdate::Parse(&_pData[1], (nDataSize - 1), eResult);
				if(_pVNCMsg){
					pVNCMsg = dynamic_cast<vncMsg*>(*_pVNCMsg);
				}
				break;
			}

		default:
			{
				eResult = vncError_InvalidData;
				OVNC_DEBUG_ERROR("%u not supported as server message", _pData[0]);
				break;
			}
	}


	return pVNCMsg;
}

vncObjectWrapper<vncMsg*> vncMsg::ParseClientMsg(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	if(!pData){
		OVNC_DEBUG_ERROR("Invalid parameter");
		eResult = vncError_InvalidParameter;
		return NULL;
	}
	if(nDataSize < 1){
		eResult = vncError_TooShort;
		return NULL;
	}
	
	const uint8_t* _pData = (uint8_t*)pData;
	vncObjectWrapper<vncMsg*> pVNCMsg;

	switch(_pData[0])
	{
		case vncRfbClientMsg_SetEncodings:
			{
				vncObjectWrapper<vncMsgSetEncoding*>_pVNCMsg = vncMsgSetEncoding::Parse(&_pData[1], (nDataSize - 1), eResult);
				if(_pVNCMsg){
					pVNCMsg = dynamic_cast<vncMsg*>(*_pVNCMsg);
				}
				break;
			}

		case vncRfbClientMsg_SetPixelFormat:
			{
				vncObjectWrapper<vncMsgSetPixelFormat*>_pVNCMsg = vncMsgSetPixelFormat::Parse(&_pData[1], (nDataSize - 1), eResult);
				if(_pVNCMsg){
					pVNCMsg = dynamic_cast<vncMsg*>(*_pVNCMsg);
				}
				break;
			}

		case vncRfbClientMsg_KeyEvent:
			{
				vncObjectWrapper<vncMsgKeyEvent*>_pVNCMsg = vncMsgKeyEvent::Parse(&_pData[1], (nDataSize - 1), eResult);
				if(_pVNCMsg){
					pVNCMsg = dynamic_cast<vncMsg*>(*_pVNCMsg);
				}
				break;
			}

		case vncRfbClientMsg_PointerEvent:
			{
				vncObjectWrapper<vncMsgPointerEvent*>_pVNCMsg = vncMsgPointerEvent::Parse(&_pData[1], (nDataSize - 1), eResult);
				if(_pVNCMsg){
					pVNCMsg = dynamic_cast<vncMsg*>(*_pVNCMsg);
				}
				break;
			}

		case vncRfbClientMsg_FramebufferUpdateRequest:
			{
				vncObjectWrapper<vncMsgFramebufferUpdateRequest*>_pVNCMsg = vncMsgFramebufferUpdateRequest::Parse(&_pData[1], (nDataSize - 1), eResult);
				if(_pVNCMsg){
					pVNCMsg = dynamic_cast<vncMsg*>(*_pVNCMsg);
				}
				break;
			}
		
	}

	return pVNCMsg;
}


//
//	vncMsgProtocolVersion
//

vncMsgProtocolVersion::vncMsgProtocolVersion(int nMajorVersion, int nMinorVersion)
:vncMsg(vncRfbMsgType_ProtocolVersion)
{
	m_nMajorVersion = nMajorVersion;
	m_nMinorVersion = nMinorVersion;
}

vncMsgProtocolVersion::~vncMsgProtocolVersion()
{

}

vncObjectWrapper<vncBuffer*> vncMsgProtocolVersion::GetBytes()
{
	OVNC_DEBUG_ERROR("Not Implemented");
	return NULL;
}

vncObjectWrapper<vncMsgProtocolVersion*> vncMsgProtocolVersion::Parse(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	if(!pData || !nDataSize){
		OVNC_DEBUG_ERROR("Invalid parameter");
		eResult = vncError_InvalidParameter;
		return NULL;
	}
	if(nDataSize < (unsigned)(vncRfbSize_ProtocolVersion)){
		OVNC_DEBUG_INFO("Too short");
		eResult = vncError_TooShort;
		return NULL;
	}

	const uint8_t* _pData = (uint8_t*)pData;
	bool isInValid = _pData[0] != 'R' || 
		_pData[1] != 'F' ||
		_pData[2] != 'B' || 
		_pData[3] != ' ' ||
		_pData[7] != '.' ||
		_pData[11] != '\n';
	if(isInValid){
		OVNC_DEBUG_ERROR("Invalid data");
		return NULL;
	}

	vncObjectWrapper<vncMsgProtocolVersion*> pVNCMsg = new vncMsgProtocolVersion();
	pVNCMsg->m_nMajorVersion = (int)vncUtils::StringToInteger((const char*)&_pData[4]);
	pVNCMsg->m_nMinorVersion = (int)vncUtils::StringToInteger((const char*)&_pData[8]);

	eResult = vncError_Ok;

	return pVNCMsg;
}



//
//	vncMsgSecurity
//

vncMsgSecurity::vncMsgSecurity()
:vncMsg(vncRfbMsgType_Security)
{
}

vncMsgSecurity::~vncMsgSecurity()
{
	
}

vncObjectWrapper<vncBuffer*> vncMsgSecurity::GetBytes()
{
	if(m_aSecurity.size() == 0){
		OVNC_DEBUG_ERROR("No sucurity type");
		return NULL;
	}
	uint8_t* pBuffer = (uint8_t*)vncUtils::Calloc(GetSize(), sizeof(uint8_t));
	if(!pBuffer){
		OVNC_DEBUG_ERROR("Failed to allocate buffer");
		return NULL;
	}

	pBuffer[0] = m_aSecurity.size();
	int i = 1;
	std::list<vncRfbSecurity_t>::iterator iter = m_aSecurity.begin();
	while(iter != m_aSecurity.end()){
		pBuffer[i++] = (vncRfbSecurity_t)(*iter);
		++iter;
	}

	vncObjectWrapper<vncBuffer*> pVNCBuffer = vncBuffer::New();
	if(pVNCBuffer){
		pVNCBuffer->TakeOwnership((void**)&pBuffer, GetSize());
	}
	OVNC_SAFE_FREE(pBuffer);

	return pVNCBuffer;
}

vncObjectWrapper<vncMsgSecurity*> vncMsgSecurity::Parse(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	if(!pData || !nDataSize){
		OVNC_DEBUG_ERROR("Invalid parameter");
		eResult = vncError_InvalidParameter;
		return NULL;
	}

	const uint8_t* _pData = (uint8_t*)pData;
	unsigned number_of_security_types = _pData[0];
	if(number_of_security_types > (nDataSize - 1)){
		OVNC_DEBUG_INFO("Too short");
		eResult = vncError_TooShort;
		return NULL;
	}
	
	vncObjectWrapper<vncMsgSecurity*> pVNCMsg = new vncMsgSecurity();
	for(unsigned i = 0; i<number_of_security_types; i++){
		pVNCMsg->m_aSecurity.push_back((vncRfbSecurity_t)_pData[i + 1]);
	}

	eResult = vncError_Ok;

	return pVNCMsg;
}



//
//	vncMsgSecurityResult
//

vncMsgSecurityResult::vncMsgSecurityResult()
:vncMsg(vncRfbMsgType_SecurityResult)
{
}

vncMsgSecurityResult::~vncMsgSecurityResult()
{
}

vncObjectWrapper<vncBuffer*> vncMsgSecurityResult::GetBytes()
{
	OVNC_DEBUG_ERROR("Not Implemented");
	return NULL;
}

vncObjectWrapper<vncMsgSecurityResult*> vncMsgSecurityResult::Parse(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	if(!pData || !nDataSize){
		OVNC_DEBUG_ERROR("Invalid parameter");
		eResult = vncError_InvalidParameter;
		return NULL;
	}

	if(nDataSize < (unsigned)vncRfbSize_SecurityResult){
		OVNC_DEBUG_INFO("Too short ");
		eResult = vncError_TooShort;
		return NULL;
	}

	vncObjectWrapper<vncMsgSecurityResult*> pVNCMsg = new vncMsgSecurityResult();
	pVNCMsg->m_eResult = (vncRfbSecurityResult_t)vncNetUtils::NetworkToHostPLong(pData);

	eResult = vncError_Ok;

	return pVNCMsg;
}




//
//	vncMsgVNCAuthChallenge
//

vncMsgVNCAuthChallenge::vncMsgVNCAuthChallenge()
:vncMsg(vncRfbMsgType_VNCAuthChallenge)
{
	memset(m_aResponse, 0, sizeof(m_aResponse));
	memset(m_aChallenge, 0, sizeof(m_aChallenge));
	m_bResponseComputed = false;
}

vncMsgVNCAuthChallenge::~vncMsgVNCAuthChallenge()
{
}

vncObjectWrapper<vncBuffer*> vncMsgVNCAuthChallenge::GetBytes()
{
	OVNC_DEBUG_ERROR("Not Implemented");
	return NULL;
}

const char* vncMsgVNCAuthChallenge::GetResponse(const char* password)
{
	if(!m_bResponseComputed){
		unsigned nResultSize = 0;
		vncError_t ret = vncUtils::D3DesEncrypt(password ? password : "", m_aChallenge, &m_aResponse[0], nResultSize);
		if(ret != vncError_Ok){
			OVNC_DEBUG_ERROR("D3DesEncrypt failed");
			return NULL;
		}
		m_bResponseComputed = true;
	}
	return m_aResponse;
}

vncObjectWrapper<vncMsgVNCAuthChallenge*> vncMsgVNCAuthChallenge::Parse(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	if(!pData || !nDataSize){
		eResult = vncError_InvalidParameter;
		OVNC_DEBUG_ERROR("Invalid parameter");
		return NULL;
	}
	if(nDataSize < (unsigned)vncRfbSize_VNCAuthChallenge){
		eResult = vncError_TooShort;
		OVNC_DEBUG_INFO("Too short");
		return NULL;
	}
	vncObjectWrapper<vncMsgVNCAuthChallenge*>pVNCMsg = new vncMsgVNCAuthChallenge();
	if(pVNCMsg){
		memcpy(pVNCMsg->m_aChallenge, pData, (unsigned)vncRfbSize_VNCAuthChallenge);
	}

	eResult = vncError_Ok;

	return pVNCMsg;
}



//
//	vncMsgServerInit
//

vncMsgServerInit::vncMsgServerInit()
:vncMsg(vncRfbMsgType_ServerInit)
{
	m_nWidth = 0;
	m_nHeight = 0;
	memset(&m_PixelFormat, 0, sizeof(vncRfbPixelFormat_t));
	m_nNameLength = 0;
	m_pName = NULL;
}

vncMsgServerInit::~vncMsgServerInit()
{
	OVNC_SAFE_FREE(m_pName);
}

vncObjectWrapper<vncBuffer*> vncMsgServerInit::GetBytes()
{
	uint8_t* pBuffer = (uint8_t*)vncUtils::Calloc(1, GetSize());
	if(!pBuffer){
		OVNC_DEBUG_ERROR("Failed to allocate buffer");
		return NULL;
	}
	
	pBuffer[0] = (m_nWidth >> 8) & 0xFF;
	pBuffer[1] = m_nWidth & 0xFF;
	pBuffer[2] = (m_nHeight >> 8) & 0xFF;
	pBuffer[3] = m_nHeight & 0xFF;
	pBuffer[4] = m_PixelFormat.bits_per_pixel;
	pBuffer[5] = m_PixelFormat.depth;
	pBuffer[6] = m_PixelFormat.big_endian_flag;
	pBuffer[7] = m_PixelFormat.true_colour_flag;
	pBuffer[8] = (m_PixelFormat.red_max >> 8) & 0xFF;
	pBuffer[9] = m_PixelFormat.red_max & 0xFF;
	pBuffer[10] = (m_PixelFormat.green_max >> 8) & 0xFF;
	pBuffer[11] = m_PixelFormat.green_max & 0xFF;
	pBuffer[12] = (m_PixelFormat.blue_max >> 8) & 0xFF;
	pBuffer[13] = m_PixelFormat.blue_max & 0xFF;
	pBuffer[14] = m_PixelFormat.red_shift;
	pBuffer[15] = m_PixelFormat.green_shift;
	pBuffer[16] = m_PixelFormat.blue_shift;
	/* Padding -> 3 bytes */
	pBuffer[20] = (m_nNameLength >> 24) & 0xFF;
	pBuffer[21] = (m_nNameLength >> 16) & 0xFF;
	pBuffer[22] = (m_nNameLength >> 8) & 0xFF;
	pBuffer[23] = (m_nNameLength) & 0xFF;
	memcpy(&pBuffer[24], m_pName, m_nNameLength);

	vncObjectWrapper<vncBuffer*> pVNCBuffer = vncBuffer::New();
	if(pVNCBuffer){
		pVNCBuffer->TakeOwnership((void**)&pBuffer, GetSize());
		OVNC_SAFE_FREE(pBuffer);
	}
	return pVNCBuffer;
}

vncObjectWrapper<vncMsgServerInit*> vncMsgServerInit::Parse(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	static unsigned g_nMiniSize = 2 + 2 + 16 + 4;
	if(!pData || !nDataSize){
		OVNC_DEBUG_ERROR("Invalid parameter");
		eResult = vncError_InvalidParameter;
		return NULL;
	}
	if(nDataSize < g_nMiniSize){
		OVNC_DEBUG_INFO("Too short");
		eResult = vncError_TooShort;
		return NULL;
	}

	const uint8_t* _pData = (uint8_t*)pData;

	uint32_t nNameLength = vncNetUtils::NetworkToHostPLong(&_pData[20]);
	if((nNameLength + g_nMiniSize) > nDataSize){
		eResult = vncError_TooShort;
		OVNC_DEBUG_INFO("Too short");
		return NULL;
	}
	uint8_t* pNameString = (uint8_t*)vncUtils::Malloc(nNameLength + 1);
	if(!pNameString){
		eResult = vncError_MemoryAllocFailed;
		OVNC_DEBUG_ERROR("Allocation of %u bytes failed", nNameLength);
		return NULL;
	}
	memcpy(pNameString, &_pData[24], nNameLength);
	pNameString[nNameLength] = '\0';
	
	vncObjectWrapper<vncMsgServerInit*> pVNCMsg = new vncMsgServerInit();
	pVNCMsg->m_nWidth = vncNetUtils::NetworkToHostPShort(&_pData[0]);
	pVNCMsg->m_nHeight = vncNetUtils::NetworkToHostPShort(&_pData[2]);

	pVNCMsg->m_PixelFormat.bits_per_pixel = _pData[4];
	pVNCMsg->m_PixelFormat.depth = _pData[5];
	pVNCMsg->m_PixelFormat.big_endian_flag = _pData[6];
	pVNCMsg->m_PixelFormat.true_colour_flag = _pData[7];
	pVNCMsg->m_PixelFormat.red_max = vncNetUtils::NetworkToHostPShort(&_pData[8]);
	pVNCMsg->m_PixelFormat.green_max = vncNetUtils::NetworkToHostPShort(&_pData[10]);
	pVNCMsg->m_PixelFormat.blue_max = vncNetUtils::NetworkToHostPShort(&_pData[12]);
	pVNCMsg->m_PixelFormat.red_shift = _pData[14];
	pVNCMsg->m_PixelFormat.green_shift = _pData[15];
	pVNCMsg->m_PixelFormat.blue_shift = _pData[16];
	memset(pVNCMsg->m_PixelFormat.padding, 0, sizeof(pVNCMsg->m_PixelFormat.padding));

	pVNCMsg->m_nNameLength = nNameLength;
	pVNCMsg->m_pName = pNameString, pNameString = NULL;
	
	eResult = vncError_Ok;

	return pVNCMsg;
}



//
//	vncMsgSetEncoding
//

vncMsgSetEncoding::vncMsgSetEncoding()
:vncMsg(vncRfbMsgType_SetEncoding)
{
}

vncMsgSetEncoding::~vncMsgSetEncoding()
{
}

vncObjectWrapper<vncBuffer*> vncMsgSetEncoding::GetBytes()
{
	uint8_t* pBuffer = (uint8_t*)vncUtils::Calloc(1, GetSize());
	if(!pBuffer){
		OVNC_DEBUG_ERROR("Failed to allocate buffer");
		return NULL;
	}

	uint8_t* _pBuffer = pBuffer;

	*_pBuffer = vncRfbClientMsg_SetEncodings, ++_pBuffer;
	*_pBuffer = 0x00, ++_pBuffer; /* Padding */
	*((uint16_t*)_pBuffer) = vncNetUtils::HostToNetworkShort(m_aEncodings.size()), _pBuffer += 2;
	std::list<vncRfbEncoding_t>::iterator iter = m_aEncodings.begin();
	for(; iter != m_aEncodings.end(); ++iter){
		*((int32_t*)_pBuffer) = vncNetUtils::HostToNetworkLong(*iter);
		_pBuffer += 4;
	}

	vncObjectWrapper<vncBuffer*> pVNCBuffer = vncBuffer::New();
	if(pVNCBuffer){
		pVNCBuffer->TakeOwnership((void**)&pBuffer, GetSize());
		OVNC_SAFE_FREE(pBuffer);
	}
	return pVNCBuffer;
}

vncObjectWrapper<vncMsgSetEncoding*> vncMsgSetEncoding::Parse(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	if(!pData){
		OVNC_DEBUG_ERROR("Invalid parameter");
		eResult = vncError_InvalidParameter;
		return NULL;
	}
	if(nDataSize < 3/*padding(1) + number-of-encodings(2)*/){
		eResult = vncError_TooShort;
		return NULL;
	}

	const uint8_t* _pData = (const uint8_t*)pData;
	unsigned _nDataSize = nDataSize;
	++_pData;//pading
	--_nDataSize;

	uint16_t nNumberOfEncodings = vncNetUtils::NetworkToHostPShort(_pData);
	_pData += 2;// number-of-encodings
	_nDataSize -= 2;

	if((nNumberOfEncodings * (unsigned)4) > _nDataSize){
		eResult = vncError_TooShort;
		return NULL;
	}

	vncObjectWrapper<vncMsgSetEncoding*>pVNCMsg = new vncMsgSetEncoding();
	if(pVNCMsg){
		eResult = vncError_Ok;
		for(uint16_t i = 0; i < nNumberOfEncodings; ++i){
			pVNCMsg->AddEncoding((vncRfbEncoding_t)vncNetUtils::NetworkToHostPLong(&_pData[i * 4]));
		}
	}
	else{
		eResult = vncError_ApplicationError;
		OVNC_DEBUG_ERROR("Failed to create vncMsgSetEncoding instance");
	}
	
	return pVNCMsg;
}



//
//	vncMsgSetPixelFormat
//


vncMsgSetPixelFormat::vncMsgSetPixelFormat()
:vncMsg(vncRfbMsgType_SetPixelFormat)
{
	memset(&m_PixelFormat, 0, sizeof(vncRfbPixelFormat_t));
}

vncMsgSetPixelFormat::~vncMsgSetPixelFormat()
{
}

	
vncError_t vncMsgSetPixelFormat::SetFormat(const vncRfbPixelFormat_t* pFormat)
{
	if(!pFormat){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	memcpy(&m_PixelFormat, pFormat, sizeof(vncRfbPixelFormat_t));
	return vncError_Ok;
}

vncObjectWrapper<vncBuffer*> vncMsgSetPixelFormat::GetBytes()
{
	uint8_t* pBuffer = (uint8_t*)vncUtils::Calloc(GetSize(), sizeof(uint8_t));
	if(!pBuffer){
		OVNC_DEBUG_ERROR("Failed to allocate buffer");
		return NULL;
	}

	pBuffer[0] = vncRfbClientMsg_SetPixelFormat;
	/* Padding => 3 Bytes */
	pBuffer[4] = m_PixelFormat.bits_per_pixel;
	pBuffer[5] = m_PixelFormat.depth;
	pBuffer[6] = m_PixelFormat.big_endian_flag;
	pBuffer[7] = m_PixelFormat.true_colour_flag;
	pBuffer[8] = (m_PixelFormat.red_max >> 8) & 0xFF;
	pBuffer[9] = m_PixelFormat.red_max & 0xFF;
	pBuffer[10] = (m_PixelFormat.green_max >> 8) & 0xFF;
	pBuffer[11] = m_PixelFormat.green_max & 0xFF;
	pBuffer[12] = (m_PixelFormat.blue_max >> 8) & 0xFF;
	pBuffer[13] = m_PixelFormat.blue_max & 0xFF;
	pBuffer[14] = m_PixelFormat.red_shift;
	pBuffer[15] = m_PixelFormat.green_shift;
	pBuffer[16] = m_PixelFormat.blue_shift;
	/* Padding => 3 Bytes */

	vncObjectWrapper<vncBuffer*> pVNCBuffer = vncBuffer::New();
	if(pVNCBuffer){
		pVNCBuffer->TakeOwnership((void**)&pBuffer, GetSize());
	}
	OVNC_SAFE_FREE(pBuffer);
	return pVNCBuffer;
}

vncObjectWrapper<vncMsgSetPixelFormat*> vncMsgSetPixelFormat::Parse(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	if(!pData){
		OVNC_DEBUG_ERROR("Invalid parameter");
		eResult = vncError_InvalidParameter;
		return NULL;
	}
	if(nDataSize < 19){
		eResult = vncError_TooShort;
		return NULL;
	}

	vncObjectWrapper<vncMsgSetPixelFormat*> pVNCMsg = new vncMsgSetPixelFormat();
	if(pVNCMsg){
		const uint8_t*_pData = ((const uint8_t*)pData) + 3/* padding */;
		pVNCMsg->m_PixelFormat.bits_per_pixel = _pData[0];
		pVNCMsg->m_PixelFormat.depth = _pData[1];
		pVNCMsg->m_PixelFormat.big_endian_flag = _pData[2];
		pVNCMsg->m_PixelFormat.true_colour_flag = _pData[3];
		pVNCMsg->m_PixelFormat.red_max = vncNetUtils::NetworkToHostPShort(&_pData[4]);
		pVNCMsg->m_PixelFormat.green_max = vncNetUtils::NetworkToHostPShort(&_pData[6]);
		pVNCMsg->m_PixelFormat.blue_max = vncNetUtils::NetworkToHostPShort(&_pData[8]);
		pVNCMsg->m_PixelFormat.red_shift = _pData[10];
		pVNCMsg->m_PixelFormat.green_shift = _pData[11];
		pVNCMsg->m_PixelFormat.blue_shift = _pData[12];
		eResult = vncError_Ok;
	}
	else{
		OVNC_DEBUG_ERROR("Failed to create vncMsgSetPixelFormat instance");
		eResult = vncError_ApplicationError;
	}

	return pVNCMsg;
}

//
//	vncMsgFramebufferUpdateRequest
//

vncMsgFramebufferUpdateRequest::vncMsgFramebufferUpdateRequest()
:vncMsg(vncRfbMsgType_FramebufferUpdateRequest)
{
	m_bIncremental = false;
	m_nXPosition = 0;
	m_nYPosition = 0;
	m_nWidth = 0;
	m_nHeight = 0;
}

vncMsgFramebufferUpdateRequest::~vncMsgFramebufferUpdateRequest()
{

}

vncObjectWrapper<vncBuffer*> vncMsgFramebufferUpdateRequest::GetBytes()
{
	uint8_t* pBuffer = (uint8_t*)vncUtils::Calloc(GetSize(), sizeof(uint8_t));
	if(!pBuffer){
		OVNC_DEBUG_ERROR("Failed to allocate buffer");
		return NULL;
	}
	
	uint8_t* _pBuffer = pBuffer;
	
	*_pBuffer = vncRfbClientMsg_FramebufferUpdateRequest, ++_pBuffer;
	*_pBuffer = m_bIncremental ? 0x01 : 0x00, ++_pBuffer;
	*((uint16_t*)_pBuffer) = vncNetUtils::HostToNetworkShort(m_nXPosition), _pBuffer += 2;
	*((uint16_t*)_pBuffer) = vncNetUtils::HostToNetworkShort(m_nYPosition), _pBuffer += 2;
	*((uint16_t*)_pBuffer) = vncNetUtils::HostToNetworkShort(m_nWidth), _pBuffer += 2;
	*((uint16_t*)_pBuffer) = vncNetUtils::HostToNetworkShort(m_nHeight), _pBuffer += 2;

	vncObjectWrapper<vncBuffer*> pVNCBuffer = vncBuffer::New();
	if(pVNCBuffer){
		pVNCBuffer->TakeOwnership((void**)&pBuffer, GetSize());
	}
	OVNC_SAFE_FREE(pBuffer);
	return pVNCBuffer;
}

vncObjectWrapper<vncMsgFramebufferUpdateRequest*> vncMsgFramebufferUpdateRequest::Parse(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	if(!pData){
		OVNC_DEBUG_ERROR("Invalid parameter");
		eResult = vncError_InvalidParameter;
		return NULL;
	}
	if(nDataSize < 9){
		eResult = vncError_TooShort;
		return NULL;
	}

	vncObjectWrapper<vncMsgFramebufferUpdateRequest*> pVNCMsg = new vncMsgFramebufferUpdateRequest();
	if(pVNCMsg){
		const uint8_t*_pData = (const uint8_t*)pData;
		pVNCMsg->SetIncremental(_pData[0] == 0x01);
		pVNCMsg->SetPosition(vncNetUtils::NetworkToHostPShort(&_pData[1]), vncNetUtils::NetworkToHostPShort(&_pData[3]));
		pVNCMsg->SetSize(vncNetUtils::NetworkToHostPShort(&_pData[5]), vncNetUtils::NetworkToHostPShort(&_pData[7]));
		eResult = vncError_Ok;
	}
	else{
		OVNC_DEBUG_ERROR("Failed to create vncMsgSetPixelFormat instance");
		eResult = vncError_ApplicationError;
	}

	return pVNCMsg;
}


//
//	vncMsgFramebufferUpdate
//

vncMsgFramebufferUpdate::vncMsgFramebufferUpdate()
:vncMsg(vncRfbMsgType_FramebufferUpdate)
{
	m_nSize = 0;
}

vncMsgFramebufferUpdate::~vncMsgFramebufferUpdate()
{
	m_Rectangles.clear();
}

vncError_t vncMsgFramebufferUpdate::AddRectangle(vncRfbRectangle_t* pRectangle, const void* pData, unsigned nDataSize)
{
	if(!pRectangle || !pData || !nDataSize){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	vncObjectWrapper<vncRfbRectangleData*>_pRectangle = vncRfbRectangleData::New(pRectangle, pData, nDataSize);
	if(!_pRectangle){
		OVNC_DEBUG_ERROR("Failed to create rectangle");
		return vncError_MemoryAllocFailed;
	}
	m_Rectangles.push_back(_pRectangle);
	return vncError_Ok;
}

vncObjectWrapper<vncBuffer*> vncMsgFramebufferUpdate::GetBytes()
{
	OVNC_DEBUG_ERROR("Not Implemented");
	return NULL;
}

vncObjectWrapper<vncMsgFramebufferUpdate*> vncMsgFramebufferUpdate::Parse(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	if(!pData){
		OVNC_DEBUG_ERROR("Invalid parameter");
		eResult = vncError_InvalidParameter;
		return NULL;
	}
	if(nDataSize < 3){
		eResult = vncError_TooShort;
		return NULL;
	}

	eResult = vncError_InvalidData;

	vncObjectWrapper<vncMsgFramebufferUpdate*>pVNCMsg;
	const uint8_t* _pData = (const uint8_t*)pData;
	unsigned _nDataSize = nDataSize;
	++_pData; // padding
	--_nDataSize;

	uint16_t number_of_rectangles = vncNetUtils::NetworkToHostPShort(_pData);
	_pData += 2; // number-of-rectangles
	_nDataSize -= 2;


	vncRfbRectangle_t rectangle;
	eResult = vncRfbUtils::ParseRectangle(_pData, _nDataSize, rectangle);
	if(eResult != vncError_Ok){
		return NULL;
	}

	switch(rectangle.encoding_type)
	{
		case vncRfbEncoding_Raw:
			{
				const uint8_t *rawData = (_pData + OVNC_RFB_RECTANGLE_HEADER_SIZE);
				unsigned rawDataSize = (_nDataSize - OVNC_RFB_RECTANGLE_HEADER_SIZE);
				if(rawDataSize < (unsigned)(rectangle.width * rectangle.height * 4)){//FIXME
					eResult = vncError_TooShort;
					return NULL;
				}

				pVNCMsg = new vncMsgFramebufferUpdate();
				pVNCMsg->m_nSize = nDataSize + 1/* message-type */;
				pVNCMsg->AddRectangle(&rectangle, rawData, rawDataSize);
				
				eResult = vncError_Success;
				break;
			}

		case vncRfbEncoding_ZRLE:
			{
				const uint8_t *zLibData = _pData;
				unsigned zLibDataSize = _nDataSize;
				for(uint16_t i = 0; i < number_of_rectangles; ++i){
					// Parse rectangle
					eResult = vncRfbUtils::ParseRectangle(zLibData, zLibDataSize, rectangle);
					if(eResult != vncError_Ok){
						return NULL;
					}
					zLibData += OVNC_RFB_RECTANGLE_HEADER_SIZE;
					zLibDataSize -= OVNC_RFB_RECTANGLE_HEADER_SIZE;
					if(zLibDataSize < 4){
						eResult = vncError_TooShort;
						return NULL;
					}
					unsigned nLength = vncNetUtils::NetworkToHostPLong(zLibData);
					zLibDataSize -= 4;
					zLibData += 4;
					if(zLibDataSize < nLength){
						eResult = vncError_TooShort;
						return NULL;
					}
					zLibDataSize -= nLength;
					zLibData += nLength;
				}
			
				pVNCMsg = new vncMsgFramebufferUpdate();
				pVNCMsg->m_nSize = nDataSize + 1/* message-type */;
				for(uint16_t i = 0; i < number_of_rectangles; ++i){
					vncRfbUtils::ParseRectangle(_pData, _nDataSize, rectangle);
					_pData += OVNC_RFB_RECTANGLE_HEADER_SIZE, _nDataSize -= OVNC_RFB_RECTANGLE_HEADER_SIZE;
					unsigned nLength = vncNetUtils::NetworkToHostPLong(_pData);
					_pData += 4, _nDataSize -= 4;
					pVNCMsg->AddRectangle(&rectangle, _pData, nLength);
					_pData += nLength, _nDataSize -= nLength;
				}
				
				eResult = vncError_Success;
				break;
			}
	}

	return pVNCMsg;
}






//
//	vncMsgPointerEvent
//

vncMsgPointerEvent::vncMsgPointerEvent(uint8_t nButtonMask, uint16_t nXPosition, uint16_t nYPosition)
:vncMsg(vncRfbMsgType_PointerEvent)
{
	m_nButtonMask = nButtonMask;
	m_nXPosition = nXPosition;
	m_nYPosition = nYPosition;
}

vncMsgPointerEvent::~vncMsgPointerEvent()
{

}

vncObjectWrapper<vncBuffer*> vncMsgPointerEvent::GetBytes()
{
	uint8_t* pBuffer = (uint8_t*)vncUtils::Calloc(GetSize(), sizeof(uint8_t));
	if(!pBuffer){
		OVNC_DEBUG_ERROR("Failed to allocate buffer");
		return NULL;
	}
		
	pBuffer[0] = vncRfbClientMsg_PointerEvent;
	pBuffer[1] = m_nButtonMask;

	pBuffer[2] = (m_nXPosition >> 8) & 0xFF;
	pBuffer[3] = m_nXPosition & 0xFF;

	pBuffer[4] = (m_nYPosition >> 8) & 0xFF;
	pBuffer[5] = m_nYPosition & 0xFF;

	vncObjectWrapper<vncBuffer*> pVNCBuffer = vncBuffer::New();
	if(pVNCBuffer){
		pVNCBuffer->TakeOwnership((void**)&pBuffer, GetSize());
	}
	OVNC_SAFE_FREE(pBuffer);

	return pVNCBuffer;
}

vncObjectWrapper<vncMsgPointerEvent*> vncMsgPointerEvent::Parse(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	if(!pData){
		OVNC_DEBUG_ERROR("Invalid parameter");
		eResult = vncError_InvalidParameter;
		return NULL;
	}
	if(nDataSize < 5){
		eResult = vncError_TooShort;
		return NULL;
	}

	vncObjectWrapper<vncMsgPointerEvent*> pVNCMsg = new vncMsgPointerEvent();
	if(pVNCMsg){
		const uint8_t*_pData = (const uint8_t*)pData;
		pVNCMsg->SetButtonMask(_pData[0]);
		pVNCMsg->SetXPosition(vncNetUtils::NetworkToHostPShort(&_pData[1]));
		pVNCMsg->SetYPosition(vncNetUtils::NetworkToHostPShort(&_pData[3]));
		eResult = vncError_Ok;
	}
	else{
		OVNC_DEBUG_ERROR("Failed to create vncMsgSetPixelFormat instance");
		eResult = vncError_ApplicationError;
	}

	return pVNCMsg;
}




//
//	vncMsgKeyEvent
//

vncMsgKeyEvent::vncMsgKeyEvent(bool bDownFlag, uint32_t nKey)
:vncMsg(vncRfbMsgType_KeyEvent)
{
	m_bDownFlag = bDownFlag;
	m_nKey = nKey;
}

vncMsgKeyEvent::~vncMsgKeyEvent()
{

}
	
vncObjectWrapper<vncBuffer*> vncMsgKeyEvent::GetBytes()
{
	uint8_t* pBuffer = (uint8_t*)vncUtils::Calloc(GetSize(), sizeof(uint8_t));
	if(!pBuffer){
		OVNC_DEBUG_ERROR("Failed to allocate buffer");
		return NULL;
	}

	pBuffer[0] = vncRfbClientMsg_KeyEvent;
	pBuffer[1] = m_bDownFlag ? 1 : 0;
	/* Padding -> 2 Bytes */
	pBuffer[4] = (m_nKey >> 24) & 0xFF;
	pBuffer[5] = (m_nKey >> 16) & 0xFF;
	pBuffer[6] = (m_nKey >> 8) & 0xFF;
	pBuffer[7] = (m_nKey) & 0xFF;

	vncObjectWrapper<vncBuffer*> pVNCBuffer = vncBuffer::New();
	if(pVNCBuffer){
		pVNCBuffer->TakeOwnership((void**)&pBuffer, GetSize());
	}
	OVNC_SAFE_FREE(pBuffer);

	return pVNCBuffer;
}

vncObjectWrapper<vncMsgKeyEvent*> vncMsgKeyEvent::Parse(const void* pData, unsigned nDataSize, vncError_t &eResult)
{
	if(!pData){
		OVNC_DEBUG_ERROR("Invalid parameter");
		eResult = vncError_InvalidParameter;
		return NULL;
	}
	if(nDataSize < 7){
		eResult = vncError_TooShort;
		return NULL;
	}

	vncObjectWrapper<vncMsgKeyEvent*> pVNCMsg = new vncMsgKeyEvent();
	if(pVNCMsg){
		const uint8_t*_pData = (const uint8_t*)pData;
		pVNCMsg->SetDownFlag(_pData[0] == 0x01);
		/* padding => 2 Bytes */
		pVNCMsg->SetKey(vncNetUtils::NetworkToHostPLong(&_pData[3]));
		eResult = vncError_Ok;
	}
	else{
		OVNC_DEBUG_ERROR("Failed to create vncMsgSetPixelFormat instance");
		eResult = vncError_ApplicationError;
	}

	return pVNCMsg;
}