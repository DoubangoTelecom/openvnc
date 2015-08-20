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
#include "openvnc/vncServer.h"
#include "openvnc/network/vncNetUtils.h"
#include "openvnc/rfb/vncRfbUtils.h"
#include "openvnc/vncUtils.h"

#define SINGLE_QUOTEME_(x) #x[0]
#define SINGLE_QUOTEME(x) SINGLE_QUOTEME_(x)

static int8_t g_aProtocolVersionMessage[12] = 
{
	'R', 'F', 'B', ' ', 
	'0', '0', SINGLE_QUOTEME(OVNC_CONFIG_VERSION_MAJOR),
	'.',
	'0', '0', SINGLE_QUOTEME(OVNC_CONFIG_VERSION_MINOR),
	'\n'
};

#include <functional>
#include <algorithm>

vncServer::vncServer(const char* pLocalHost, vncNetPort_t nLocalPort, vncNetworkType_t eType)
:vncRunnable(vncServer::Run)
{
	m_bValid = false;
	m_eState = vncServerState_None;
	m_pPassword = NULL;

	// Supported encodings (by priority order)
	m_aEncodings.push_back(vncRfbEncoding_ZRLE);
	m_aEncodings.push_back(vncRfbEncoding_Raw);


	m_pVNCTransport = vncNetTransport::New(pLocalHost, nLocalPort, eType, "Doubango VNC Server");
	if(m_pVNCTransport){
		m_pVNCTransport->SetCallback(vncServer::NetworkCallback, this);
	}
	m_pVNCPeersMutex = vncMutex::New(false);
	m_bValid = (m_pVNCTransport && m_pVNCTransport->IsValid() && m_pVNCPeersMutex);
}

vncServer::~vncServer()
{
	OVNC_SAFE_FREE(m_pPassword);
	vncObjectSafeFree(m_pVNCTransport);

	m_pVNCPeersMutex->Lock();
	m_Peers.clear();
	m_pVNCPeersMutex->UnLock();

	vncObjectSafeFree(m_pVNCPeersMutex);
}

vncError_t vncServer::SetPassword(const char* pPassword)
{
	return vncUtils::StringUpdate(&m_pPassword, pPassword);
}

vncError_t vncServer::AddEncoding(vncRfbEncoding_t eEncoding, bool bBack)
{
	std::list<vncRfbEncoding_t>::iterator iter = std::find(m_aEncodings.begin(), m_aEncodings.end(), eEncoding);
	if(iter == m_aEncodings.end()){
		if(bBack){
			m_aEncodings.push_back(eEncoding);
		}
		else{
			m_aEncodings.push_front(eEncoding);
		}
	}
	return vncError_Ok;
}

vncError_t vncServer::RemoveEncoding(vncRfbEncoding_t eEncoding)
{
	std::list<vncRfbEncoding_t>::iterator iter = std::find(m_aEncodings.begin(), m_aEncodings.end(), eEncoding);
	if(iter != m_aEncodings.end()){
		m_aEncodings.erase(iter);
	}
	return vncError_Ok;
}

vncError_t vncServer::ClearEncoding(vncRfbEncoding_t eEncoding)
{
	m_aEncodings.clear();
	return vncError_Ok;
}

vncError_t vncServer::Start()
{
	vncError_t ret = vncRunnable::Start();
	if(ret == vncError_Ok){
		ret = m_pVNCTransport->Start();
	}
	return ret;
}

vncError_t vncServer::Stop()
{
	vncError_t ret = vncRunnable::Stop();
	if(ret == vncError_Ok){
		ret = m_pVNCTransport->Stop();
	}
	return ret;
}

vncError_t vncServer::SendMessage(vncObjectWrapper<vncServerPeer*> pVNCPeer, vncObjectWrapper<vncMsg*>pVNCMsg)
{
	if(!pVNCPeer || !pVNCMsg){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}

	vncObjectWrapper<vncBuffer*>pVNCBuffer = pVNCMsg->GetBytes();
	if(!pVNCBuffer){
		OVNC_DEBUG_ERROR("Failed to get bytes");
		return vncError_ApplicationError;
	}
	unsigned nSentBytes = vncNetUtils::SockfdSend(pVNCPeer->GetFd(), pVNCBuffer->GetData(), pVNCBuffer->GetDataSize(), 0);
	if(nSentBytes == pVNCBuffer->GetDataSize()){
		return vncError_Ok;
	}
	return vncError_NetworkError;
}

vncError_t vncServer::SendProtocolVersion(vncObjectWrapper<vncServerPeer*> pVNCPeer)
{
	if(!pVNCPeer){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	unsigned nSentBytes = vncNetUtils::SockfdSend(pVNCPeer->GetFd(), g_aProtocolVersionMessage, sizeof(g_aProtocolVersionMessage), 0);
	if(nSentBytes == sizeof(g_aProtocolVersionMessage)){
		return vncError_Ok;
	}
	return vncError_NetworkError;
}

vncError_t vncServer::SendSecurities(vncObjectWrapper<vncServerPeer*> pVNCPeer)
{
	if(!pVNCPeer){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	vncObjectWrapper<vncMsgSecurity*>pVNCMsg = new vncMsgSecurity();
	if(!pVNCMsg){
		OVNC_DEBUG_ERROR("Failed to create vncMsgSecurity object");
		return vncError_ApplicationError;
	}
	pVNCMsg->AddSecurity(vncRfbSecurity_VNC_Authentication);
	pVNCMsg->AddSecurity(vncRfbSecurity_None);
	return SendMessage(pVNCPeer, dynamic_cast<vncMsg*>(*pVNCMsg));
}

vncError_t vncServer::SendRandom16Bytes(vncObjectWrapper<vncServerPeer*> pVNCPeer)
{
	if(!pVNCPeer){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	unsigned nSentBytes = vncNetUtils::SockfdSend(pVNCPeer->GetFd(), pVNCPeer->GetRandom16Bytes(), 16, 0);
	if(nSentBytes == 16){
		return vncError_Ok;
	}
	return vncError_NetworkError;
}

vncError_t vncServer::SendSecurityResult(vncObjectWrapper<vncServerPeer*> pVNCPeer, vncRfbSecurityResult_t eResult)
{
	if(!pVNCPeer){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	uint32_t nResult = vncNetUtils::HostToNetworkLong((unsigned long)eResult);
	unsigned nSentBytes = vncNetUtils::SockfdSend(pVNCPeer->GetFd(), &nResult, sizeof(nResult), 0);
	if(nSentBytes == sizeof(nResult)){
		return vncError_Ok;
	}
	return vncError_NetworkError;
}

vncError_t vncServer::SendFailureReason(vncObjectWrapper<vncServerPeer*> pVNCPeer, const char* pReason)
{
	if(!pVNCPeer || !pReason){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	uint32_t nReasonLength = vncUtils::StringLength(pReason);
	uint32_t nReasonLengthInNetworkOrder = vncNetUtils::HostToNetworkLong(nReasonLength);
	unsigned nSentBytes = vncNetUtils::SockfdSend(pVNCPeer->GetFd(), &nReasonLengthInNetworkOrder, sizeof(nReasonLengthInNetworkOrder), 0);
	if(nSentBytes != sizeof(nReasonLengthInNetworkOrder)){
		return vncError_NetworkError;
	}
	nSentBytes = vncNetUtils::SockfdSend(pVNCPeer->GetFd(), pReason, nReasonLength, 0);
	if(nSentBytes != nReasonLength){
		return vncError_NetworkError;
	}
	return vncError_Ok;
}

vncError_t vncServer::SendServerInit(vncObjectWrapper<vncServerPeer*> pVNCPeer)
{
	vncObjectWrapper<vncScreenGrabber*> pVNCScreenGrabber;

	if(!pVNCPeer){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	if((pVNCPeer->SetScreenGrabber(vncScreenGrabber::New())) != vncError_Ok){
		OVNC_DEBUG_ERROR("Failed to set screen grabber");
		return vncError_ApplicationError;
	}
	vncObjectWrapper<vncMsgServerInit*>pVNCMsg = new vncMsgServerInit();
	if(!pVNCMsg){
		OVNC_DEBUG_ERROR("Failed to create vncMsgServerInit object");
		return vncError_ApplicationError;
	}
	
	pVNCMsg->SetWidth(pVNCPeer->GetScreenGrabber()->GetWidth());
	pVNCMsg->SetHeight(pVNCPeer->GetScreenGrabber()->GetHeight());
	pVNCMsg->SetName("Mamadou's PC");//FIXME
	
	vncRfbPixelFormat_t pixelFormat;
	pixelFormat.bits_per_pixel = 32;
	pixelFormat.depth = 24;
	pixelFormat.big_endian_flag = vncNetUtils::IsBigEndian() ? 1 : 0;
	pixelFormat.true_colour_flag = 1;
	pixelFormat.red_max = (1<<8) - 1;
	pixelFormat.green_max = (1<<8) - 1;
	pixelFormat.blue_max = (1<<8) - 1;
	pixelFormat.red_shift = 0;
	pixelFormat.green_shift = 8;
	pixelFormat.blue_shift = 16;
	pVNCMsg->SetPixelFormat(&pixelFormat);

	// Will be updated by the remote peer not supported
	pVNCPeer->SetPixelFormat(&pixelFormat);

	return SendMessage(pVNCPeer, dynamic_cast<vncMsg*>(*pVNCMsg));
}

bool vncServer::IsChallengeResponseOk(vncObjectWrapper<vncServerPeer*> pVNCPeer, const int8_t* pResponse, unsigned nResponseSize)
{
	if(!pVNCPeer || !pResponse || nResponseSize < 16){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return false;
	}
	
	unsigned nResultSize = 0;
	int8_t aResult[16];
	vncError_t ret = vncUtils::D3DesEncrypt(m_pPassword ? m_pPassword : "", (const char*)pVNCPeer->GetRandom16Bytes(), (char*)&aResult[0], nResultSize);
	if(ret != vncError_Ok){
		OVNC_DEBUG_ERROR("D3DesEncrypt failed");
		return false;
	}
	for(int i = 0; i < sizeof(aResult); ++i){
		if(aResult[i] != pResponse[i]){
			OVNC_DEBUG_INFO("Invalid response from peer %d", pVNCPeer->GetFd());
			return false;
		}
	}
	return true;
}

vncError_t vncServer::HandleClientMessage(vncObjectWrapper<vncServerPeer*> pVNCPeer, vncObjectWrapper<vncMsg*>pVNCMsg)
{
	if(!pVNCPeer || !pVNCMsg){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}

	switch(pVNCMsg->GetType())
	{
		case vncRfbMsgType_SetPixelFormat:
			{
				vncObjectWrapper<vncMsgSetPixelFormat*>_pVNCMsg = dynamic_cast<vncMsgSetPixelFormat*>(*pVNCMsg);
				return pVNCPeer->SetPixelFormat(_pVNCMsg->GetFormat());
			}

		case vncRfbMsgType_SetEncoding:
			{
				vncObjectWrapper<vncMsgSetEncoding*>_pVNCMsg = dynamic_cast<vncMsgSetEncoding*>(*pVNCMsg);
				// Set Neg encoding (priority order)
				std::list<vncRfbEncoding_t>::const_iterator iter = _pVNCMsg->GetEncodings()->begin();
				while(iter != _pVNCMsg->GetEncodings()->end()){
					if(std::find(m_aEncodings.begin(), m_aEncodings.end(), *iter) != m_aEncodings.end()){
						OVNC_DEBUG_INFO("Negotiated encoding=%d", *iter);
						pVNCPeer->SetNegEncoding(*iter);
						break;
					}
					++iter;
				}
				return pVNCPeer->SetEncodings(_pVNCMsg->GetEncodings());
			}

		case vncRfbMsgType_FramebufferUpdateRequest:
			{
				if(!pVNCPeer->GetScreenGrabber()){
					OVNC_DEBUG_ERROR("Not video grabber associated");
					return vncError_ApplicationError;
				}
				vncObjectWrapper<vncMsgFramebufferUpdateRequest*>_pVNCMsg = dynamic_cast<vncMsgFramebufferUpdateRequest*>(*pVNCMsg);
				vncRfbRectangle_t rectangle = 
				{
					_pVNCMsg->GetXPosition(),
					_pVNCMsg->GetYPosition(),
					_pVNCMsg->GetWidth(),
					_pVNCMsg->GetHeight(),
					pVNCPeer->GetNegEncoding()
				};
				
				const void* pOutBuffer = NULL;
				unsigned nOutBufferSize = 0;
				vncError_t eRet = pVNCPeer->GetScreenGrabber()->Capture(pVNCPeer->GetPixelFormat(), &rectangle, &pOutBuffer, nOutBufferSize);
				if(eRet == vncError_Ok && pOutBuffer && nOutBufferSize){
					unsigned nSentBytes;
					// Send header

					// message-type
					uint8_t HEADER[20];
					HEADER[0] = vncRfbServerMsg_FramebufferUpdate;
					// padding
					HEADER[1] = 0x00;
					// number-of-rectangles
					HEADER[2] = 0x00;
					HEADER[3] = 0x01;
					// x-position
					HEADER[4] = (rectangle.x_position >> 8) & 0xFF;
					HEADER[5] = rectangle.x_position & 0xFF;
					// y-position
					HEADER[6] = (rectangle.y_position >> 8) & 0xFF;
					HEADER[7] = rectangle.y_position & 0xFF;
					// width
					HEADER[8] = (rectangle.width >> 8) & 0xFF;
					HEADER[9] = rectangle.width & 0xFF;
					// height
					HEADER[10] = (rectangle.height >> 8) & 0xFF;
					HEADER[11] = rectangle.height & 0xFF;
					// encoding-type
					HEADER[12] = (rectangle.encoding_type >> 24) & 0xFF;
					HEADER[13] = (rectangle.encoding_type >> 16) & 0xFF;
					HEADER[14] = (rectangle.encoding_type >> 8) & 0xFF;
					HEADER[15] = rectangle.encoding_type & 0xFF;
					switch(rectangle.encoding_type){
						case vncRfbEncoding_Raw:
						default:
							{
								nSentBytes = vncNetUtils::SockfdSend(pVNCPeer->GetFd(), HEADER, 16, 0);
								break;
							}
						case vncRfbEncoding_ZRLE:
						case vncRfbEncoding_Hextile:
						case vncRfbEncoding_RRE:
							{
								// 4 bytes length
								HEADER[16] = (nOutBufferSize >> 24) & 0xFF;
								HEADER[17] = (nOutBufferSize >> 16) & 0xFF;
								HEADER[18] = (nOutBufferSize >> 8) & 0xFF;
								HEADER[19] = nOutBufferSize & 0xFF;
								nSentBytes = vncNetUtils::SockfdSend(pVNCPeer->GetFd(), HEADER, 20, 0);
								break;
							}
					}

					// Send encoded data
					nSentBytes = vncNetUtils::SockfdSend(pVNCPeer->GetFd(), pOutBuffer, nOutBufferSize, 0);
					//eRet = (nSentBytes == nOutBufferSize) ? vncError_Ok : vncError_NetworkError;
					//if(eRet != vncError_Ok){
					//	OVNC_DEBUG_ERROR("Failed to send %u bytes", nOutBufferSize);
					//}
				}
				return eRet;
			}

		case vncRfbMsgType_PointerEvent:
			{
				if(!pVNCPeer->GetScreenGrabber()){
					OVNC_DEBUG_ERROR("Not video grabber associated");
					return vncError_ApplicationError;
				}
				vncObjectWrapper<vncMsgPointerEvent*>_pVNCMsg = dynamic_cast<vncMsgPointerEvent*>(*pVNCMsg);
				return vncError_Ok;
			}

		case vncRfbMsgType_KeyEvent:
			{
				if(!pVNCPeer->GetScreenGrabber()){
					OVNC_DEBUG_ERROR("Not video grabber associated");
					return vncError_ApplicationError;
				}
				vncObjectWrapper<vncMsgKeyEvent*>_pVNCMsg = dynamic_cast<vncMsgKeyEvent*>(*pVNCMsg);
				return vncError_Ok;
			}

		default:
			{
				OVNC_DEBUG_ERROR("Invalid message type: %d", pVNCMsg->GetType());
				return vncError_InvalidData;
			}
	}
}

vncObjectWrapper<vncServer*> vncServer::New(const char* pLocalHost, vncNetPort_t nLocalPort, vncNetworkType_t eType)
{
	vncObjectWrapper<vncServer*> pVNCServer = new vncServer(pLocalHost, nLocalPort, eType);
	if(pVNCServer && !pVNCServer->IsValid()){
		vncObjectSafeFree(pVNCServer);
	}
	return pVNCServer;
}

void* vncServer::Run(void* _This)
{
	vncServer* This = (vncServer*)_This;
	vncObjectWrapper<vncObject*> curr;
	
	OVNC_DEBUG_INFO("vncServer::Run::Enter");

	OVNC_RUNNABLE_RUN_BEGIN(This);
	
	if((curr = This->PopFirstObject())){
		/*vncObjectWrapper<vncServertEvent*> pVNCEvent = dynamic_cast<vncServertEvent*>(*curr);
		if(pVNCEvent && This->m_pcCallback){
			This->m_pcCallback->OnEvent(*pVNCEvent);
		}*/
		
#if I_WANT_MY_MEMORY_RIGHT_NOW
		vncObjectSafeRelease(curr);
		vncObjectSafeRelease(pVNCEvent);
#endif
	}

	OVNC_RUNNABLE_RUN_END(This);

	OVNC_DEBUG_INFO("vncServer::Run::Exit");

	return NULL;
}

int vncServer::NetworkCallback(const vncNetTransportEvent* pcEvent)
{
	if(!pcEvent){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return -1;
	}

	vncObjectWrapper<vncServer*>This = (vncServer*)pcEvent->GetContext();

	switch(pcEvent->GetType())
	{
		case  vncNetTransportEventType_Accepted:
			{
				// Send Server Protocol version
				vncObjectWrapper<vncServerPeer*>pVNCPeer = new vncServerPeer(pcEvent->GetFd());
				if(This->SendProtocolVersion(pVNCPeer) == vncError_Ok){
					This->m_pVNCPeersMutex->Lock();
					pVNCPeer->SetState(vncServerPeerState_ProtocolVersionSent);
					This->m_Peers.insert(std::pair<vncNetFd_t, vncObjectWrapper<vncServerPeer*> >(pVNCPeer->GetFd(), pVNCPeer));
					This->m_pVNCPeersMutex->UnLock();
				}
				else{
					This->m_pVNCTransport->RemoveSocket(pcEvent->GetFd());
				}
				break;
			}

		case vncNetTransportEventType_Data:
			{
				vncError_t eResult;

				This->m_pVNCPeersMutex->Lock();
				std::map<vncNetFd_t, vncObjectWrapper<vncServerPeer*> >::iterator iter = This->m_Peers.find(pcEvent->GetFd());
				if(iter == This->m_Peers.end()){
					This->m_pVNCPeersMutex->UnLock();
					OVNC_DEBUG_ERROR("Failed to find peer with id = %d", pcEvent->GetFd());
					This->m_pVNCTransport->RemoveSocket(pcEvent->GetFd());
					return -1;
				}
				vncObjectWrapper<vncServerPeer*>pVNCPeer = iter->second;
				pVNCPeer->GetBuffer()->Append(pcEvent->GetDataPtr(), pcEvent->GetDataSize());
				switch(pVNCPeer->GetState())
				{
					case vncServerPeerState_ProtocolVersionSent:
						{
							vncObjectWrapper<vncMsgProtocolVersion*>pVNCMsg = vncMsgProtocolVersion::Parse(pVNCPeer->GetBuffer()->GetData(), pVNCPeer->GetBuffer()->GetDataSize(), eResult);
							if(pVNCMsg){
								pVNCPeer->GetBuffer()->Remove(0, (unsigned)pVNCMsg->GetSize());
								int majorVersion = pVNCMsg->GetMajorVersion();
								int minorVersion = pVNCMsg->GetMinorVersion();
								if(majorVersion < OVNC_CONFIG_VERSION_MAJOR_MIN || (majorVersion == OVNC_CONFIG_VERSION_MAJOR_MIN && minorVersion < OVNC_CONFIG_VERSION_MINOR_MIN)){
									OVNC_DEBUG_ERROR("Server Version(Major=%d,Minor=%d) not supported", majorVersion, minorVersion);
									pVNCPeer->SetState(vncServerPeerState_ProtocolVersionNOk);
									This->m_pVNCTransport->RemoveSocket(pcEvent->GetFd());
								}
								else{
									pVNCPeer->SetVersion(majorVersion, minorVersion);
									OVNC_DEBUG_INFO("Peer Version(Major=%d,Minor=%d)", majorVersion, minorVersion);
									pVNCPeer->SetState(vncServerPeerState_ProtocolVersionOk);
									// Send SecurityTypes
									if((eResult = This->SendSecurities(pVNCPeer)) != vncError_Ok){
										This->m_pVNCPeersMutex->UnLock();
										This->m_pVNCTransport->RemoveSocket(pcEvent->GetFd());
										return -1;
									}
									pVNCPeer->SetState(vncServerPeerState_SecuritiesSent);
								}
							}
							break;
						}

					case vncServerPeerState_SecuritiesSent:
						{
							if(pVNCPeer->GetBuffer()->GetDataSize() >= 1){
								vncRfbSecurity_t eSecurity = (vncRfbSecurity_t)*((uint8_t*)pVNCPeer->GetBuffer()->GetData());
								pVNCPeer->GetBuffer()->Remove(0, 1);
								if(/*FIXME: eSecurity not supported*/false){
									pVNCPeer->SetState(vncServerPeerState_SecuritiesNOk);
									This->m_pVNCPeersMutex->UnLock();
									This->m_pVNCTransport->RemoveSocket(pcEvent->GetFd());
									return -1;
								}
								pVNCPeer->SetSecurity(eSecurity);
								pVNCPeer->SetState(vncServerPeerState_SecuritiesOk);

								if(pVNCPeer->GetSecurity() == vncRfbSecurity_VNC_Authentication){
									if((eResult = This->SendRandom16Bytes(pVNCPeer)) != vncError_Ok){
										pVNCPeer->SetState(vncServerPeerState_SecuritiesNOk);
										This->m_pVNCPeersMutex->UnLock();
										This->m_pVNCTransport->RemoveSocket(pcEvent->GetFd());
										return -1;
									}
									pVNCPeer->SetState(vncServerPeerState_ChallengeSent);
								}
							}
							break;
						}

					case vncServerPeerState_ChallengeSent:
						{
							if(pVNCPeer->GetBuffer()->GetDataSize() >= 16){
								bool bChallengeResponseIsOk = This->IsChallengeResponseOk(pVNCPeer, (const int8_t*)pVNCPeer->GetBuffer()->GetData(), 16);
								pVNCPeer->GetBuffer()->Remove(0, 16);
								if(!bChallengeResponseIsOk){
									eResult = This->SendSecurityResult(pVNCPeer, vncRfbSecurityResult_Failed);
									eResult = This->SendFailureReason(pVNCPeer, "Invalid password!");
									pVNCPeer->SetState(vncServerPeerState_ChallengeNOk);
									This->m_pVNCPeersMutex->UnLock();
									This->m_pVNCTransport->RemoveSocket(pcEvent->GetFd());
									return -1;
								}
								if((eResult = This->SendSecurityResult(pVNCPeer, vncRfbSecurityResult_Ok)) == vncError_Ok){
									pVNCPeer->SetState(vncServerPeerState_ChallengeOk);
								}
								else{
									pVNCPeer->SetState(vncServerPeerState_ChallengeNOk);
									This->m_pVNCPeersMutex->UnLock();
									This->m_pVNCTransport->RemoveSocket(pcEvent->GetFd());
									return -1;
								}
							}
							break;
						}

					case vncServerPeerState_ChallengeOk:
						{
							// ClientInit
							if(pVNCPeer->GetBuffer()->GetDataSize() >= 1){
								pVNCPeer->SetSharedFlag(*((uint8_t*)pVNCPeer->GetBuffer()->GetData()) == 1);
								pVNCPeer->GetBuffer()->Remove(0, 1);
								pVNCPeer->SetState(vncServerPeerState_ClientInitReceived);
								// Send ServerInit message
								if((eResult = This->SendServerInit(pVNCPeer)) != vncError_Ok){
									pVNCPeer->SetState(vncServerPeerState_NetworkError);
									This->m_pVNCPeersMutex->UnLock();
									This->m_pVNCTransport->RemoveSocket(pcEvent->GetFd());
									return -1;
								}
								pVNCPeer->SetState(vncServerPeerState_SessionEstablished);
							}
							break;
						}

					case vncServerPeerState_SessionEstablished:
						{
							do
							{
								vncObjectWrapper<vncMsg*>pVNCMsg = vncMsg::ParseClientMsg(pVNCPeer->GetBuffer()->GetData(), pVNCPeer->GetBuffer()->GetDataSize(), eResult);
								if(eResult != vncError_Ok && eResult != vncError_TooShort){
									pVNCPeer->SetState(vncServerPeerState_InvalidData);
									This->m_pVNCPeersMutex->UnLock();
									This->m_pVNCTransport->RemoveSocket(pcEvent->GetFd());
									return -1;
								}
								if(pVNCMsg && eResult == vncError_Ok){
									pVNCPeer->GetBuffer()->Remove(0, pVNCMsg->GetSize());
									if((eResult = This->HandleClientMessage(pVNCPeer, pVNCMsg)) != vncError_Ok){
										pVNCPeer->SetState(vncServerPeerState_InvalidData);
										This->m_pVNCPeersMutex->UnLock();
										This->m_pVNCTransport->RemoveSocket(pcEvent->GetFd());
										return -1;
									}
								}
							}
							while(eResult == vncError_Ok && pVNCPeer->GetBuffer()->GetDataSize() > 0);

							break;
						}

					case vncServerPeerState_None:
					default:
						{
							break;
						}
				}

				This->m_pVNCPeersMutex->UnLock();
				break;

			}// EndOfDataEvent

		case vncNetTransportEventType_Closed:
			{
				This->m_pVNCPeersMutex->Lock();
				std::map<vncNetFd_t, vncObjectWrapper<vncServerPeer*> >::iterator iter = This->m_Peers.find(pcEvent->GetFd());
				if(iter != This->m_Peers.end()){
					This->m_Peers.erase(iter);
				}				
				This->m_pVNCPeersMutex->UnLock();
				break;
			}
	}

	return 0;
}