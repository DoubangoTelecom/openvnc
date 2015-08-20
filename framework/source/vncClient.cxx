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
#include "openvnc/vncClient.h"
#include "openvnc/rfb/vncRfbMsg.h"
#include "openvnc/rfb/vncRfbUtils.h"
#include "openvnc/network/vncNetUtils.h"

#define SATE_MACHINE_DEBUG (true)

typedef enum vncClientAction_e
{
	vncClientAction_Configure,
	vncClientAction_Start,
	vncClientAction_Connect,
	vncClientAction_Connected,
	vncClientAction_ProtocolVersionOk,
	vncClientAction_ProtocolVersionNOk,
	vncClientAction_SecurityNegOk,
	vncClientAction_SecurityNegNOk,
	vncClientAction_ChallengeOk,
	vncClientAction_ChallengeNOk,
	vncClientAction_ServerInitOk,
	vncClientAction_ServerInitNOk,
	vncClientAction_Disconnect,
	vncClientAction_Disconnected,
	vncClientAction_Stop
}
vncClientAction_t;

#define VNC_CLIENT_EXEC_ACTION(This, action, ...)\
	This->m_pVNCStateMachine->ExecuteAction((ovnc_fsm_action_id_t)action, NULL, NULL, \
		This, ##__VA_ARGS__)

vncClient::vncClient(const char* pLocalHost, vncNetPort_t nLocalPort, vncNetworkType_t eType)
:vncRunnable(vncClient::Run)
{
	m_pParentWindow = NULL;
	m_pcCallback = NULL;

	m_eState = vncClientState_None;
	m_bValid = false;
	m_bIncrement = true;
	m_nServerFD = OVNC_NET_INVALID_FD;
	m_pPassword = NULL;
	m_pServerHost = NULL;
	m_nServerPort = OVNC_CONFIG_SERVER_PORT;
	m_eType = vncNetworkType_TCP;
	m_eIPVersion = vncNetIPVersion_IPv4and6;
	m_Version.major = OVNC_CONFIG_VERSION_MAJOR;
	m_Version.minor = OVNC_CONFIG_VERSION_MINOR;
	m_eSecurity = vncRfbSecurity_None;
	memset(m_aChallengResponse, 0, sizeof(m_aChallengResponse));
	
	m_Client.bSharedFlag = true;
	memset(&m_Client.PixelFormat, 0, sizeof(vncRfbPixelFormat_t));

	m_Server.nWidth = 0;
	m_Server.nHeight = 0;
	memset(&m_Server.PixelFormat, 0, sizeof(vncRfbPixelFormat_t));
	m_Server.nNameLength = 0;
	m_Server.pName = NULL;

	m_pVNCBuffer = vncBuffer::New();

	m_pVNCMutex = vncMutex::New(false);

	m_pVNCTransport = vncNetTransport::New(pLocalHost, nLocalPort, eType, "Doubango VNC Client");
	if(m_pVNCTransport){
		m_pVNCTransport->SetCallback(vncClient::NetworkCallback, this);
	}
	m_pVNCStateMachine = vncStateMachine::New(vncClientState_None, vncClientState_Terminated);
	vncError_t retFsm = vncError_Ok;
	if(m_pVNCStateMachine){
		m_pVNCStateMachine->SetDebug(SATE_MACHINE_DEBUG);
		m_pVNCStateMachine->SetTermIfExecFail(false);

		retFsm = vncStateMachine::Set(m_pVNCStateMachine,
			OVNC_FSM_ADD_ALWAYS(vncClientState_None, vncClientAction_Configure, vncClientState_Configured, &Fsm_None_2_Configured_X_Configure, "Fsm_None_2_Configured_X_Configure"),
			OVNC_FSM_ADD_ALWAYS(vncClientState_Configured, vncClientAction_Start, vncClientState_Started, &Fsm_Configured_2_Started_X_Start, "Fsm_Configured_2_Started_X_Start"),
			OVNC_FSM_ADD_ALWAYS(vncClientState_Started, vncClientAction_Connect, vncClientState_Connecting, &Fsm_Started_2_Connecting_X_Connect, "Fsm_Started_2_Connecting_X_Connect"),
			OVNC_FSM_ADD_ALWAYS(vncClientState_Connecting, vncClientAction_Connected, vncClientState_Connected, &Fsm_Connecting_2_Connected_X_Connected, "Fsm_Connecting_2_Connected_X_Connected"),
			
			OVNC_FSM_ADD_ALWAYS(vncClientState_Connected, vncClientAction_ProtocolVersionOk, vncClientState_ProtocolVersionNegotiated, &Fsm_Connected_2_ProtocolVersionNegotiated_X_ProtocolVersionOk, "Fsm_Connected_2_ProtocolVersionNegotiated_X_ProtocolVersionOk"),
			OVNC_FSM_ADD_ALWAYS(vncClientState_Connected, vncClientAction_ProtocolVersionNOk, vncClientState_Terminated, &Fsm_Connected_2_Terminated_X_ProtocolVersionNOk, "Fsm_Connected_2_Terminated_X_ProtocolVersionNOk"),
			
			OVNC_FSM_ADD_ALWAYS(vncClientState_ProtocolVersionNegotiated, vncClientAction_SecurityNegOk, vncClientState_SecurityNegotiating, &Fsm_ProtocolVersionNegotiated_2_SecurityNegotiating_X_SecurityNegOk, "Fsm_ProtocolVersionNegotiated_2_SecurityNegotiating_X_SecurityNegOk"),
			OVNC_FSM_ADD_ALWAYS(vncClientState_ProtocolVersionNegotiated, vncClientAction_SecurityNegNOk, vncClientState_Terminated, &Fsm_ProtocolVersionNegotiated_2_Terminated_X_SecurityNegNOk, "Fsm_ProtocolVersionNegotiated_2_Terminated_X_SecurityNegNOk"),

			OVNC_FSM_ADD_ALWAYS(vncClientState_SecurityNegotiating, vncClientAction_ChallengeOk, vncClientState_SecurityChallenging, &Fsm_SecurityNegotiating_2_SecurityChallenging_X_ChallengeOk, "Fsm_SecurityNegotiating_2_SecurityChallenging_X_ChallengeOk"),
			OVNC_FSM_ADD_ALWAYS(vncClientState_SecurityNegotiating, vncClientAction_ChallengeNOk, vncClientState_Terminated, &Fsm_SecurityNegotiating_2_Terminated_X_ChallengeNOk, "Fsm_SecurityNegotiating_2_Terminated_X_ChallengeNOk"),

			OVNC_FSM_ADD_ALWAYS(vncClientState_SecurityChallenging, vncClientAction_ChallengeOk, vncClientState_SecurityChallenged, &Fsm_SecurityChallenging_2_SecurityChallenged_X_ChallengeOk, "Fsm_SecurityChallenging_2_SecurityChallenged_X_ChallengeOk"),
			OVNC_FSM_ADD_ALWAYS(vncClientState_SecurityChallenging, vncClientAction_ChallengeNOk, vncClientState_Terminated, &Fsm_SecurityChallenging_2_Terminated_X_ChallengeNOk, "Fsm_SecurityChallenging_2_Terminated_X_ChallengeNOk"),

			OVNC_FSM_ADD_ALWAYS(vncClientState_SecurityChallenged, vncClientAction_ServerInitOk, vncClientState_SessionEstablished, &Fsm_SecurityChallenged_2_SessionEstablished_X_ServerInitOk, "Fsm_SecurityChallenged_2_SessionEstablished_X_ServerInitOk"),
			OVNC_FSM_ADD_ALWAYS(vncClientState_SecurityChallenged, vncClientAction_ServerInitNOk, vncClientState_Terminated, &Fsm_SecurityChallenged_2_Terminated_X_ServerInitOk, "Fsm_SecurityChallenged_2_Terminated_X_ServerInitOk"),

			OVNC_FSM_ADD_ALWAYS(ovnc_fsm_state_any, vncClientAction_Disconnect, vncClientState_Terminated, &Fsm_Any_2_Terminated_X_Disconnect, "Fsm_Any_2_Terminated_X_Disconnect"),
			OVNC_FSM_ADD_ALWAYS(ovnc_fsm_state_any, vncClientAction_Disconnected, vncClientState_Terminated, &Fsm_Any_2_Terminated_X_Disconnected, "Fsm_Any_2_Terminated_X_Disconnected"),
			
			OVNC_FSM_ADD_NULL()
			);
	}	
	
	m_bValid = (m_pVNCTransport && m_pVNCTransport->IsValid() && m_pVNCStateMachine && (retFsm == vncError_Ok) && m_pVNCBuffer);
}

vncClient::~vncClient()
{
	vncObjectSafeFree(m_pVNCTransport);
	vncObjectSafeFree(m_pVNCStateMachine);
	vncObjectSafeFree(m_pVNCBuffer);
	vncObjectSafeFree(m_pVNCDisplay);
	OVNC_SAFE_FREE(m_pPassword);
	OVNC_SAFE_FREE(m_pServerHost);
	OVNC_SAFE_FREE(m_Server.pName);
}

vncError_t vncClient::SetPassword(const char* pPassword)
{
	return vncUtils::StringUpdate(&m_pPassword, pPassword);
}

vncError_t vncClient::SetPixelFormat(const vncRfbPixelFormat_t* pPixelFormat)
{
	if(!pPixelFormat){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}

	memcpy(&m_Client.PixelFormat, pPixelFormat, sizeof(m_Client.PixelFormat));

	if(IsConnected()){
		vncObjectWrapper<vncMsgSetPixelFormat*>pVNCMsg = new vncMsgSetPixelFormat();
		if(pVNCMsg){
			if(vncRfbUtils::IsSet(&m_Client.PixelFormat)){
				pVNCMsg->SetFormat(&m_Client.PixelFormat);
			}
			else{
				pVNCMsg->SetFormat(&m_Server.PixelFormat);
			}
			vncObjectWrapper<vncBuffer*>pVNCBuffer = pVNCMsg->GetBytes();
			if(pVNCBuffer){
				unsigned nSentBytes = vncNetUtils::SockfdSend(m_nServerFD, pVNCBuffer->GetData(), pVNCBuffer->GetDataSize(), 0);
				return (nSentBytes == pVNCBuffer->GetDataSize()) ? vncError_Ok : vncError_NetworkError;
			}
			else{
				vncError_ApplicationError;
			}
		}
	}
	return vncError_Ok;
}

vncError_t vncClient::SetPixelSize(vncRfbPixelSize_t ePixelSize)
{
	vncRfbPixelFormat_t pixelFormat;

	switch(ePixelSize)
	{
		case vncRfbPixelType_8:
			{
				pixelFormat.bits_per_pixel = 8;
				pixelFormat.depth = 8;
				pixelFormat.big_endian_flag = vncNetUtils::IsBigEndian() ? 1 : 0;
				pixelFormat.true_colour_flag = 1;
				pixelFormat.red_max = (1<<3) - 1;
				pixelFormat.green_max = (1<<3) - 1;
				pixelFormat.blue_max = (1<<2) - 1;
				pixelFormat.red_shift = 0;
				pixelFormat.green_shift = 3;
				pixelFormat.blue_shift = 6;
				break;
			}

		case vncRfbPixelType_16:
			{
				pixelFormat.bits_per_pixel = 16;
				pixelFormat.depth = 16;
				pixelFormat.big_endian_flag = vncNetUtils::IsBigEndian() ? 1 : 0;
				pixelFormat.true_colour_flag = 1;
				pixelFormat.red_max = (1<<5) - 1;
				pixelFormat.green_max = (1<<6) - 1;
				pixelFormat.blue_max = (1<<5) - 1;
				pixelFormat.red_shift = 0;
				pixelFormat.green_shift = 5;
				pixelFormat.blue_shift = 11;
				break;
			}

		case vncRfbPixelType_32:
			{
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
				break;
			}

		case vncRfbPixelType_None:
		default:
			{
				vncRfbUtils::UnSet(&pixelFormat);
				break;
			}
	}

	return SetPixelFormat(&pixelFormat);
}

vncError_t vncClient::Configure(const char* pServerHost, vncNetPort_t nServerPort, vncNetworkType_t eType, vncNetIPVersion_t eIPVersion)
{
	return VNC_CLIENT_EXEC_ACTION(this, vncClientAction_Configure, pServerHost, nServerPort, eType, eIPVersion);
}

vncError_t vncClient::Start()
{
	vncError_t ret = vncRunnable::Start();
	if(ret == vncError_Ok){
		ret = VNC_CLIENT_EXEC_ACTION(this, vncClientAction_Start);
	}
	return ret;
}

vncError_t vncClient::Connect()
{
	return VNC_CLIENT_EXEC_ACTION(this, vncClientAction_Connect);
}

vncError_t vncClient::Disconnect()
{
	if(m_nServerFD == OVNC_NET_INVALID_FD){
		OVNC_DEBUG_ERROR("Not connected");
		return vncError_InvalidState;
	}

	return VNC_CLIENT_EXEC_ACTION(this, vncClientAction_Disconnect);
}

bool vncClient::IsConnected()
{
	switch(m_eState){
		case vncClientState_Connected:
		case vncClientState_ProtocolVersionNegotiated:
		case vncClientState_SecurityNegotiating:
		case vncClientState_SecurityChallenging:
		case vncClientState_SecurityChallenged:
		case vncClientState_SessionEstablished:
			return true;
		default:
			return false;
	}
}

bool vncClient::IsSessionEstablished()
{
	return (m_eState == vncClientState_SessionEstablished);
}

vncError_t vncClient::SetParentWindow(void* pParentWindow)
{
	m_pParentWindow = pParentWindow;
	return vncError_Ok;
}

vncError_t vncClient::SetParentWindow(int64_t parentWindow)
{
	return SetParentWindow((void*)parentWindow);
}

vncError_t vncClient::SetCallback(const vncClientCallback* pcCallback)
{
	m_pcCallback = pcCallback;
	return vncError_Ok;
}

vncError_t vncClient::SendPointerEvent(uint8_t nButtonMask, uint16_t nXPosition, uint16_t nYPosition)
{
	if(!IsConnected()){
		OVNC_DEBUG_ERROR("Not connected");
		return vncError_InvalidState;
	}
	
	if(m_pVNCDisplay){
		m_pVNCDisplay->AdjustMousePointerToFit(nXPosition, nYPosition);
	}

	vncObjectWrapper<vncMsgPointerEvent*>pVNCMsg = new vncMsgPointerEvent(nButtonMask, nXPosition, nYPosition);
	if(!pVNCMsg){
		OVNC_DEBUG_ERROR("Failed to create message");
		return vncError_MemoryAllocFailed;
	}
	
	vncObjectWrapper<vncBuffer*> pVNCBuffer = pVNCMsg->GetBytes();
	if(!pVNCBuffer){
		OVNC_DEBUG_ERROR("Null buffer");
		return vncError_ApplicationError;
	}
	unsigned nSentBytes = vncNetUtils::SockfdSend(m_nServerFD, pVNCBuffer->GetData(), pVNCBuffer->GetDataSize(), 0);
	return (nSentBytes == pVNCBuffer->GetDataSize()) ? vncError_Ok : vncError_NetworkError;
}

vncError_t vncClient::SendKeyEvent(bool bDownFlag, uint32_t nKey)
{
	if(!IsConnected()){
		OVNC_DEBUG_ERROR("Not connected");
		return vncError_InvalidState;
	}
	
	vncObjectWrapper<vncMsgKeyEvent*>pVNCMsg = new vncMsgKeyEvent(bDownFlag, nKey);
	if(!pVNCMsg){
		OVNC_DEBUG_ERROR("Failed to create message");
		return vncError_MemoryAllocFailed;
	}
	
	vncObjectWrapper<vncBuffer*> pVNCBuffer = pVNCMsg->GetBytes();
	if(!pVNCBuffer){
		OVNC_DEBUG_ERROR("Null buffer");
		return vncError_ApplicationError;
	}
	unsigned nSentBytes = vncNetUtils::SockfdSend(m_nServerFD, pVNCBuffer->GetData(), pVNCBuffer->GetDataSize(), 0);
	return (nSentBytes == pVNCBuffer->GetDataSize()) ? vncError_Ok : vncError_NetworkError;
}

//
// Final State Machine Transitions
//
vncError_t vncClient::Fsm_None_2_Configured_X_Configure(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);
	const char* pServerHost = va_arg(*app, const char*);
	vncNetPort_t nServerPort = (vncNetPort_t)va_arg(*app, unsigned);
	vncNetworkType_t eType = va_arg(*app, vncNetworkType_t);
	vncNetIPVersion_t eIPVersion = va_arg(*app, vncNetIPVersion_t);

	OVNC_SAFE_FREE(This->m_pServerHost);
	This->m_pServerHost = vncUtils::StringDup(pServerHost);
	This->m_nServerPort = nServerPort;
	This->m_eType = eType;
	This->m_eIPVersion = eIPVersion;

	return vncError_Ok;
}

vncError_t vncClient::Fsm_Configured_2_Started_X_Start(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);
	if(!This->m_pVNCTransport){
		OVNC_DEBUG_ERROR("Invalid state");
		return vncError_InvalidState;
	}

	This->m_pVNCMutex->Lock();

	vncError_t ret = This->m_pVNCTransport->Start();
	if(ret == vncError_Ok){
		This->SetState(vncClientState_Started);
	}

	This->m_pVNCMutex->UnLock();
	
	return ret;
}

vncError_t vncClient::Fsm_Started_2_Connecting_X_Connect(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);
	if(This->m_nServerFD != OVNC_NET_INVALID_FD){
		OVNC_DEBUG_ERROR("Already connected");
		return vncError_InvalidState;
	}

	This->m_pVNCMutex->Lock();

	This->m_nServerFD =  This->m_pVNCTransport->ConnectTo(This->m_pServerHost, This->m_nServerPort, This->m_eType, This->m_eIPVersion);
	vncError_t ret = (This->m_nServerFD == OVNC_NET_INVALID_FD) ? vncError_NetworkError : vncError_Ok;
	if(ret == vncError_Ok){
		This->SetState(vncClientState_Connecting);
	}

	This->m_pVNCMutex->UnLock();

	return ret;
}

vncError_t vncClient::Fsm_Connecting_2_Connected_X_Connected(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);
	//vncError_t ret = vncNetUtils::SockFdListen(This->m_nServerFD, OVNC_NET_MAX_FDS);
	//if(ret != vncError_Ok){
	//	vncUtils_SystemPrintLastError("Failed to listen on socket FD");
	//	return ret;
	//}
	This->m_pVNCMutex->Lock();

	This->SetState(vncClientState_Connected);

	This->m_pVNCMutex->UnLock();

	return vncError_Ok;
}

vncError_t vncClient::Fsm_Connected_2_ProtocolVersionNegotiated_X_ProtocolVersionOk(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);
	vncError_t ret;
	vncNumberString_t majorVersion;
	vncNumberString_t minorVersion;
	if((ret = vncUtils::IntegerToString(This->m_Version.major, &majorVersion)) != vncError_Ok){
		OVNC_DEBUG_ERROR("Failed to convert number to string");
		return ret;
	}
	if((ret = vncUtils::IntegerToString(This->m_Version.minor, &minorVersion))){
		OVNC_DEBUG_ERROR("Failed to convert number to string");
		return ret;
	}
	int8_t aProtocolVersionMessage[12] = 
	{
		'R', 'F', 'B', ' ', 
		'0', '0', majorVersion[0],
		'.',
		'0', '0', minorVersion[0],
		'\n'
	};

	This->m_pVNCMutex->Lock();

	unsigned nSentBytes = vncNetUtils::SockfdSend(This->m_nServerFD, aProtocolVersionMessage, sizeof(aProtocolVersionMessage), 0);

	ret = (nSentBytes == sizeof(aProtocolVersionMessage)) ? vncError_Ok : vncError_NetworkError;

	if(ret == vncError_Ok){
		This->SetState(vncClientState_ProtocolVersionNegotiated);
	}

	This->m_pVNCMutex->UnLock();

	return ret;
}

vncError_t vncClient::Fsm_Connected_2_Terminated_X_ProtocolVersionNOk(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);
	return vncError_Ok;
}

vncError_t vncClient::Fsm_ProtocolVersionNegotiated_2_SecurityNegotiating_X_SecurityNegOk(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);

	This->m_pVNCMutex->Lock();
	
	unsigned nSentBytes = vncNetUtils::SockfdSend(This->m_nServerFD, (int8_t*)&This->m_eSecurity, sizeof(int8_t), 0);
	vncError_t ret = (nSentBytes == sizeof(int8_t)) ? vncError_Ok : vncError_NetworkError;
	if(ret == vncError_Ok){
		This->SetState(vncClientState_SecurityNegotiating);
	}

	This->m_pVNCMutex->UnLock();

	return ret;
}

vncError_t vncClient::Fsm_ProtocolVersionNegotiated_2_Terminated_X_SecurityNegNOk(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);
	return vncError_Ok;
}

vncError_t vncClient::Fsm_SecurityNegotiating_2_SecurityChallenging_X_ChallengeOk(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);

	This->m_pVNCMutex->Lock();

	// send chared key and wait for SecurityResult
	unsigned nSentBytes = vncNetUtils::SockfdSend(This->m_nServerFD, This->m_aChallengResponse, sizeof(This->m_aChallengResponse), 0);
	vncError_t ret = (nSentBytes == sizeof(This->m_aChallengResponse)) ? vncError_Ok : vncError_NetworkError;
	if(ret == vncError_Ok){
		This->SetState(vncClientState_SecurityChallenging);
	}

	This->m_pVNCMutex->UnLock();

	return ret;
}

vncError_t vncClient::Fsm_SecurityNegotiating_2_Terminated_X_ChallengeNOk(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);
	return vncError_Ok;
}

vncError_t vncClient::Fsm_SecurityChallenging_2_SecurityChallenged_X_ChallengeOk(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);
	
	static int8_t g_nOne = 1;
	static int8_t g_nZero = 0;

	This->m_pVNCMutex->Lock();

	// Send ClientInit
	unsigned nSentBytes = vncNetUtils::SockfdSend(This->m_nServerFD, This->m_Client.bSharedFlag ? &g_nOne : &g_nZero, 1, 0);
	vncError_t ret = (nSentBytes == 1) ? vncError_Ok : vncError_NetworkError;
	if(ret == vncError_Ok){
		This->SetState(vncClientState_SecurityChallenged);
	}

	This->m_pVNCMutex->UnLock();

	return ret;
}

vncError_t vncClient::Fsm_SecurityChallenging_2_Terminated_X_ChallengeNOk(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);
	return vncError_Ok;
}

vncError_t vncClient::Fsm_SecurityChallenged_2_SessionEstablished_X_ServerInitOk(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);
	vncObjectWrapper<vncBuffer*>pVNCBuffer;

	This->m_pVNCMutex->Lock();

	// Send SetEncoding
	vncObjectWrapper<vncMsgSetEncoding*>pVNCMsg = new vncMsgSetEncoding();
	vncError_t ret = vncError_Ok;
	if(pVNCMsg){
		pVNCMsg->AddEncoding(vncRfbEncoding_ZRLE);
		pVNCMsg->AddEncoding(vncRfbEncoding_Raw);
		
		pVNCBuffer = pVNCMsg->GetBytes();
		if(pVNCBuffer){
			unsigned nSentBytes = vncNetUtils::SockfdSend(This->m_nServerFD, pVNCBuffer->GetData(), pVNCBuffer->GetDataSize(), 0);
			ret = (nSentBytes == pVNCBuffer->GetDataSize()) ? vncError_Ok : vncError_NetworkError;
		}
		else{
			OVNC_DEBUG_ERROR("Failed to create VNC buffer");
			ret = vncError_MemoryAllocFailed;
		}
	}
	else{
		OVNC_DEBUG_ERROR("Failed to create VNC message");
		ret = vncError_MemoryAllocFailed;
	}
	
	// Send PixelFormat
	if(ret == vncError_Ok && vncRfbUtils::IsSet(&This->m_Client.PixelFormat)){
		vncObjectWrapper<vncMsgSetPixelFormat*>pVNCMsg = new vncMsgSetPixelFormat();
		if(pVNCMsg){
			pVNCMsg->SetFormat(&This->m_Client.PixelFormat);
			pVNCBuffer = pVNCMsg->GetBytes();
			if(pVNCBuffer){
				unsigned nSentBytes = vncNetUtils::SockfdSend(This->m_nServerFD, pVNCBuffer->GetData(), pVNCBuffer->GetDataSize(), 0);
				ret = (nSentBytes == pVNCBuffer->GetDataSize()) ? vncError_Ok : vncError_NetworkError;
			}
			else{
				OVNC_DEBUG_ERROR("Failed to create VNC buffer");
				ret = vncError_MemoryAllocFailed;
			}
		}
	}

	// Send FramebufferUpdateRequest
	if(ret == vncError_Ok){
		vncObjectWrapper<vncMsgFramebufferUpdateRequest*>pVNCMsgFBUR = new vncMsgFramebufferUpdateRequest();
		if(pVNCMsgFBUR){
			pVNCMsgFBUR->SetIncremental(This->m_bIncrement);
			pVNCMsgFBUR->SetPosition(0, 0);
			pVNCMsgFBUR->SetSize(This->m_Server.nWidth, This->m_Server.nHeight);
			pVNCBuffer = pVNCMsgFBUR->GetBytes();
			if(pVNCBuffer){
				unsigned nSentBytes = vncNetUtils::SockfdSend(This->m_nServerFD, pVNCBuffer->GetData(), pVNCBuffer->GetDataSize(), 0);
				ret = (nSentBytes == pVNCBuffer->GetDataSize()) ? vncError_Ok : vncError_NetworkError;
			}
			else{
				OVNC_DEBUG_ERROR("Failed to create VNC buffer");
				ret = vncError_MemoryAllocFailed;
			}
		}
	}
	if(ret == vncError_Ok){
		This->SetState(vncClientState_SessionEstablished);
	}

	This->m_pVNCMutex->UnLock();

	return ret;
}

vncError_t vncClient::Fsm_SecurityChallenged_2_Terminated_X_ServerInitOk(va_list *app)
{
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);
	return vncError_Ok;
}

vncError_t vncClient::Fsm_Any_2_Terminated_X_Disconnect(va_list *app)
{
	vncError_t ret = vncError_Ok;
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);

	This->m_pVNCMutex->Lock();

	if(This->m_pVNCTransport && This->m_nServerFD != OVNC_NET_INVALID_FD){
		ret = This->m_pVNCTransport->RemoveSocket(This->m_nServerFD);
	}

	This->m_pVNCMutex->UnLock();

	return ret;
}

vncError_t vncClient::Fsm_Any_2_Terminated_X_Disconnected(va_list *app)
{
	vncError_t ret = vncError_Ok;
	vncObjectWrapper<vncClient*> This = va_arg(*app, vncClient*);

	This->m_pVNCMutex->Lock();

	This->m_nServerFD = OVNC_NET_INVALID_FD;
	This->SetState(vncClientState_Disconnected);
	This->m_pVNCMutex->UnLock();

	return ret;
}

vncObjectWrapper<vncClient*> vncClient::New(const char* pLocalHost, vncNetPort_t nLocalPort, vncNetworkType_t eType)
{
	vncObjectWrapper<vncClient*> pVNCClient = new vncClient(pLocalHost, nLocalPort, eType);
	if(pVNCClient && !pVNCClient->IsValid()){
		vncObjectSafeFree(pVNCClient);
	}
	return pVNCClient;
}

int vncClient::NetworkCallback(const vncNetTransportEvent* pcEvent)
{
	if(!pcEvent){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return -1;
	}

	vncObjectWrapper<vncClient*>This = (vncClient*)pcEvent->GetContext();
	int ret = 0;

	This->m_pVNCMutex->Lock();

	switch(pcEvent->GetType())
	{
		case vncNetTransportEventType_Data:
			{
				vncError_t eResult = vncError_Ok;
				if((This->m_pVNCBuffer->Append(pcEvent->GetDataPtr(), pcEvent->GetDataSize())) != vncError_Ok){
					ret = -2;
					goto bail;
				}
				if((This->m_eState == vncClientState_Connected) || This->m_eState == vncClientState_Connecting){
					// Protocol version
					vncObjectWrapper<vncMsgProtocolVersion*>pVNCMsg = vncMsgProtocolVersion::Parse(This->m_pVNCBuffer->GetData(), This->m_pVNCBuffer->GetDataSize(), eResult);
					if(pVNCMsg){
						This->m_pVNCBuffer->Remove(0, (unsigned)pVNCMsg->GetSize());
						int majorVersion = pVNCMsg->GetMajorVersion();
						int minorVersion = pVNCMsg->GetMinorVersion();
						if(majorVersion < OVNC_CONFIG_VERSION_MAJOR_MIN || (majorVersion == OVNC_CONFIG_VERSION_MAJOR_MIN && minorVersion < OVNC_CONFIG_VERSION_MINOR_MIN)){
							OVNC_DEBUG_ERROR("Server Version(Major=%d,Minor=%d) not supported", majorVersion, minorVersion);
							VNC_CLIENT_EXEC_ACTION((*This), vncClientAction_ProtocolVersionNOk);
						}
						else{
							This->m_Version.major = OVNC_CONFIG_VERSION_MAJOR; //OVNC_MIN(majorVersion, OVNC_CONFIG_VERSION_MAJOR_MAX);
							This->m_Version.minor = OVNC_CONFIG_VERSION_MINOR; //OVNC_MIN(minorVersion, OVNC_CONFIG_VERSION_MINOR_MAX);
							OVNC_DEBUG_INFO("Server Version(Major=%d,Minor=%d)", majorVersion, minorVersion);
							VNC_CLIENT_EXEC_ACTION((*This), vncClientAction_ProtocolVersionOk);
						}
					}
				}
				else if(This->m_eState == vncClientState_ProtocolVersionNegotiated){
					vncObjectWrapper<vncMsgSecurity*>pVNCMsg = vncMsgSecurity::Parse(This->m_pVNCBuffer->GetData(), This->m_pVNCBuffer->GetDataSize(), eResult);
					bool bSecurityMatched = false;
					if(pVNCMsg){
						This->m_pVNCBuffer->Remove(0, (unsigned)pVNCMsg->GetSize());
						std::list<vncRfbSecurity_t>::iterator iter;
						for(iter = pVNCMsg->GetSecurities()->begin(); iter != pVNCMsg->GetSecurities()->end(); ++iter){
							switch((*iter)){
								case vncRfbSecurity_None:
								case vncRfbSecurity_VNC_Authentication:
									{
										This->m_eSecurity = (*iter);
										bSecurityMatched = true;
										break;
									}
							}
							// VNC_Authentication is the preferred one
							if(bSecurityMatched && This->m_eSecurity == vncRfbSecurity_VNC_Authentication){
								break;
							}
						}
						if(bSecurityMatched){
							OVNC_DEBUG_INFO("Security negotiation matched=%d", This->m_eSecurity);
							VNC_CLIENT_EXEC_ACTION((*This), vncClientAction_SecurityNegOk);
						}
						else if(eResult == vncError_Ok){
							OVNC_DEBUG_ERROR("Security negotiation didn't matched");
							VNC_CLIENT_EXEC_ACTION((*This), vncClientAction_SecurityNegNOk);
						}
					}
				}
				else if(This->m_eState == vncClientState_SecurityNegotiating){
					vncObjectWrapper<vncMsgVNCAuthChallenge*>pVNCMsg = vncMsgVNCAuthChallenge::Parse(This->m_pVNCBuffer->GetData(), This->m_pVNCBuffer->GetDataSize(), eResult);
					bool bChallengeOk = false;
					if(pVNCMsg){
						This->m_pVNCBuffer->Remove(0, (unsigned)pVNCMsg->GetSize());
						const char* pResponse = pVNCMsg->GetResponse(This->m_pPassword);
						if(pResponse){
							bChallengeOk = true;
							memcpy(This->m_aChallengResponse, pResponse, OVNC_MIN(pVNCMsg->GetResponseSize(), sizeof(This->m_aChallengResponse)));
							VNC_CLIENT_EXEC_ACTION((*This), vncClientAction_ChallengeOk);
						}
					}
					if(!bChallengeOk && (eResult == vncError_Ok)){
						OVNC_DEBUG_ERROR("Invalid shared key");
						VNC_CLIENT_EXEC_ACTION((*This), vncClientAction_ChallengeNOk);
					}
				}
				else if(This->m_eState == vncClientState_SecurityChallenging){
					vncObjectWrapper<vncMsgSecurityResult*>pVNCMsg = vncMsgSecurityResult::Parse(This->m_pVNCBuffer->GetData(), This->m_pVNCBuffer->GetDataSize(), eResult);
					bool bSecurityResultOk = false;
					if(pVNCMsg){
						This->m_pVNCBuffer->Remove(0, (unsigned)pVNCMsg->GetSize());
						bSecurityResultOk = (pVNCMsg->GetResult() == vncRfbSecurityResult_Ok);
					}
					if(bSecurityResultOk){
						OVNC_DEBUG_INFO("SecurityResult challenging ok=%d", This->m_eSecurity);
						VNC_CLIENT_EXEC_ACTION((*This), vncClientAction_ChallengeOk);
					}
					else if(eResult == vncError_Ok){
						OVNC_DEBUG_ERROR("SecurityResult challenging nok");
						VNC_CLIENT_EXEC_ACTION((*This), vncClientAction_ChallengeNOk);
					}
				}
				else if(This->m_eState == vncClientState_SecurityChallenged){
					vncObjectWrapper<vncMsgServerInit*>pVNCMsg = vncMsgServerInit::Parse(This->m_pVNCBuffer->GetData(), This->m_pVNCBuffer->GetDataSize(), eResult);
					if(pVNCMsg){
						This->m_pVNCBuffer->Remove(0, (unsigned)pVNCMsg->GetSize());
						This->m_Server.nWidth = pVNCMsg->GetWidth();
						This->m_Server.nHeight = pVNCMsg->GetHeight();
						This->m_Server.nNameLength = pVNCMsg->GetNameLength();
						if(This->m_Server.nNameLength){
							vncUtils::StringUpdate((char**)&This->m_Server.pName, (const char*)pVNCMsg->GetName());
						}
						memcpy(&This->m_Server.PixelFormat, pVNCMsg->GetPixelFormat(), OVNC_MIN(pVNCMsg->GetPixelFormatSize(), sizeof(vncRfbPixelFormat_t)));

						This->m_pVNCDisplay = vncDisplay::New(This->m_Server.nWidth, This->m_Server.nHeight, This->m_pParentWindow);

						VNC_CLIENT_EXEC_ACTION((*This), vncClientAction_ServerInitOk);
					}
					else if(eResult != vncError_TooShort){
						OVNC_DEBUG_ERROR("Failed to parse ServerInit message");
						VNC_CLIENT_EXEC_ACTION((*This), vncClientAction_ServerInitNOk);
					}
				}
				else if(This->m_eState == vncClientState_SessionEstablished){					
					vncObjectWrapper<vncMsg*>pVNCMsg = vncMsg::ParseServerMsg(This->m_pVNCBuffer->GetData(), This->m_pVNCBuffer->GetDataSize(), eResult);					
					if(pVNCMsg && ret == vncError_Ok){
						This->m_pVNCBuffer->Remove(0, (unsigned)pVNCMsg->GetSize());
						OVNC_DEBUG_INFO("Done!");
						
						// --- FIXME
						if(pVNCMsg->GetType() == vncRfbMsgType_FramebufferUpdate){
							vncObjectWrapper<vncMsgFramebufferUpdate*>_pVNCMsg = dynamic_cast<vncMsgFramebufferUpdate*>(*pVNCMsg);
							std::list<vncObjectWrapper<vncRfbRectangleData*> >*pRectangles = _pVNCMsg->GetRectangles();
							if(This->m_pVNCDisplay && pRectangles){
								std::list<vncObjectWrapper<vncRfbRectangleData*> >::iterator iter = pRectangles->begin();
								while(iter != pRectangles->end()){
									vncObjectWrapper<vncVideoFrame*> pVNCFrame = new vncVideoFrame(vncRfbUtils::IsSet(&This->m_Client.PixelFormat) ? &This->m_Client.PixelFormat : &This->m_Server.PixelFormat, 
										(*iter)->GetData(), (*iter)->GetRectangle());
									This->m_pVNCDisplay->Display(pVNCFrame);
									++iter;
								}
							}
							

							
							// FIXME: Request New
							vncObjectWrapper<vncMsgFramebufferUpdateRequest*>pVNCMsgFBUR = new vncMsgFramebufferUpdateRequest();
							if(pVNCMsgFBUR){
								pVNCMsgFBUR->SetIncremental(true);
								pVNCMsgFBUR->SetPosition(0, 0);
								pVNCMsgFBUR->SetSize(This->m_Server.nWidth, This->m_Server.nHeight);
								vncObjectWrapper<vncBuffer*>pVNCBuffer = pVNCMsgFBUR->GetBytes();
								if(pVNCBuffer){
									vncNetUtils::SockfdSend(This->m_nServerFD, pVNCBuffer->GetData(), pVNCBuffer->GetDataSize(), 0);
								}
							}
						}
						// ----
					}
					else if(eResult != vncError_TooShort){
						OVNC_DEBUG_ERROR("Failed to parse server message");
						This->m_pVNCBuffer->CleanUp();
					}
					
					OVNC_DEBUG_INFO("BufferSize=%u", This->m_pVNCBuffer->GetDataSize());
				}

				break;
			}
		case vncNetTransportEventType_Closed:
			{
				if(pcEvent->GetFd() == This->m_nServerFD){
					OVNC_DEBUG_INFO("Disconnected");
					VNC_CLIENT_EXEC_ACTION((*This), vncClientAction_Disconnected);
				}
				break;
			}
		case vncNetTransportEventType_Error:
			{
				break;
			}
		case vncNetTransportEventType_Connected:
			{
				VNC_CLIENT_EXEC_ACTION((*This), vncClientAction_Connected);
				break;
			}
		case vncNetTransportEventType_Accepted:
			{
				break;
			}

		case vncNetTransportEventType_None:
		default:
			{
				break;
			}
	}

bail:

	This->m_pVNCMutex->UnLock();

	return ret;
}

void* vncClient::Run(void* _This)
{
	vncClient* This = (vncClient*)_This;
	vncObjectWrapper<vncObject*> curr;
	
	OVNC_DEBUG_INFO("vncClient::Run::Enter");

	OVNC_RUNNABLE_RUN_BEGIN(This);

	if((curr = This->PopFirstObject())){
		vncObjectWrapper<vncClientEvent*> pVNCEvent = dynamic_cast<vncClientEvent*>(*curr);
		if(pVNCEvent && This->m_pcCallback){
			This->m_pcCallback->OnEvent(*pVNCEvent);
		}

#if I_WANT_MY_MEMORY_RIGHT_NOW
		vncObjectSafeRelease(curr);
		vncObjectSafeRelease(pVNCEvent);
#endif
	}

	OVNC_RUNNABLE_RUN_END(This);

	OVNC_DEBUG_INFO("vncClient::Run::Exit");

	return NULL;
}

void vncClient::RaiseEvent(vncClientEventType_t eType)
{
	if(m_pcCallback){
		vncObjectWrapper<vncClientEvent*>pVNCEvent = new vncClientEvent(eType);
		if(pVNCEvent){
			Enqueue(dynamic_cast<vncObject*>(*pVNCEvent));
		}
	}
}

void vncClient::SetState(vncClientState_t eNewState)
{
	m_eState = eNewState;
	RaiseEvent(vncClientEventType_StateChanged);
}