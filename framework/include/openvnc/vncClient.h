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
#ifndef OVNC_CLIENT_H
#define OVNC_CLIENT_H

#include "vncConfig.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncObject.h"
#include "openvnc/network/vncNetTransport.h"
#include "openvnc/vncStateMachine.h"
#include "openvnc/vncBuffer.h"
#include "openvnc/rfb/vncRfbTypes.h"
#include "openvnc/video/vncDisplay.h"
#include "openvnc/vncRunnable.h"
#include "openvnc/vncMutex.h"

class vncClient;
class vncClientEvent;

typedef int (*ovnc_client_cb_f)(const vncClientEvent* pcEvent);

typedef enum vncClientState_e
{
	vncClientState_None = 0,

	vncClientState_Configured,
	vncClientState_Started,
	vncClientState_Connecting,
	vncClientState_Connected,
	vncClientState_ProtocolVersionNegotiated,
	vncClientState_SecurityNegotiating,
	vncClientState_SecurityChallenging,
	vncClientState_SecurityChallenged,
	vncClientState_SessionEstablished,
	vncClientState_Disconnected,
	vncClientState_Terminated
}
vncClientState_t;

typedef enum vncClientEventType_e
{
	vncClientEventType_None = 0,

	vncClientEventType_StateChanged,
}
vncClientEventType_t;




class OVNC_API vncClientEvent : public vncObject
{
public:
	vncClientEvent(vncClientEventType_t eType)
	{
		m_eType = eType;
	}

	virtual ~vncClientEvent()
	{
		
	}
	virtual OVNC_INLINE const char* GetObjectId() { return "vncClientEvent"; }
	
	// Do not not use "GetType" => reserved work in C#
	virtual OVNC_INLINE vncClientEventType_t GetEventType(){ return m_eType; }

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable : 4251 )
#endif

private:
	vncClientEventType_t m_eType;

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
};




class OVNC_API vncClientCallback
{
public:
	vncClientCallback()
	{
	}
	virtual ~vncClientCallback()
	{
	}
	virtual vncError_t OnEvent(const vncClientEvent* pcEvent)const = 0;
};




class OVNC_API vncClient : public vncRunnable
{
protected:
	vncClient(const char* pLocalHost = OVNC_NET_SOCKET_HOST_ANY, vncNetPort_t nLocalPort = OVNC_CONFIG_SERVER_PORT, vncNetworkType_t eType = vncNetworkType_TCP);
public:
	virtual ~vncClient();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncClient"; }
	virtual OVNC_INLINE bool IsValid(){ return m_bValid; }
	virtual OVNC_INLINE vncClientState_t GetSate(){ return m_eState; }
	virtual vncError_t SetPassword(const char* pPassword);
	virtual vncError_t SetPixelSize(vncRfbPixelSize_t ePixelSize);
	virtual vncError_t SetPixelFormat(const vncRfbPixelFormat_t* pPixelFormat);
	virtual vncError_t Configure(const char* pServerHost, vncNetPort_t nServerPort, vncNetworkType_t eType, vncNetIPVersion_t eIPVersion=vncNetIPVersion_IPv4and6);
	virtual vncError_t Start();
	virtual vncError_t Connect();
	virtual vncError_t Disconnect();
	virtual bool IsConnected();
	virtual bool IsSessionEstablished();
	virtual vncError_t SetParentWindow(void* pParentWindow);
	virtual vncError_t SetParentWindow(int64_t parentWindow);
	virtual vncError_t SetCallback(const vncClientCallback* pcCallback);
	
	virtual vncError_t SendPointerEvent(uint8_t nButtonMask, uint16_t nXPosition, uint16_t nYPosition);
	virtual vncError_t SendKeyEvent(bool bDownFlag, uint32_t nKey);

	static vncObjectWrapper<vncClient*> New(const char* pLocalHost = OVNC_NET_SOCKET_HOST_ANY, vncNetPort_t nLocalPort = OVNC_CONFIG_SERVER_PORT, vncNetworkType_t eType = vncNetworkType_TCP);
	
	vncClient* ABCD(){ return NULL; }

private:
	static void* Run(void* _This);
	static int NetworkCallback(const vncNetTransportEvent* pcEvent);
	void RaiseEvent(vncClientEventType_t eType);
	void SetState(vncClientState_t eNewState);

	//
	// Final State Machine Transitions
	//
	static vncError_t Fsm_None_2_Configured_X_Configure(va_list *app);
	static vncError_t Fsm_Configured_2_Started_X_Start(va_list *app);
	static vncError_t Fsm_Started_2_Connecting_X_Connect(va_list *app);
	static vncError_t Fsm_Connecting_2_Connected_X_Connected(va_list *app);
	static vncError_t Fsm_Connected_2_ProtocolVersionNegotiated_X_ProtocolVersionOk(va_list *app);
	static vncError_t Fsm_Connected_2_Terminated_X_ProtocolVersionNOk(va_list *app);
	static vncError_t Fsm_ProtocolVersionNegotiated_2_SecurityNegotiating_X_SecurityNegOk(va_list *app);
	static vncError_t Fsm_ProtocolVersionNegotiated_2_Terminated_X_SecurityNegNOk(va_list *app);
	static vncError_t Fsm_SecurityNegotiating_2_SecurityChallenging_X_ChallengeOk(va_list *app);
	static vncError_t Fsm_SecurityNegotiating_2_Terminated_X_ChallengeNOk(va_list *app);
	static vncError_t Fsm_SecurityChallenging_2_SecurityChallenged_X_ChallengeOk(va_list *app);
	static vncError_t Fsm_SecurityChallenging_2_Terminated_X_ChallengeNOk(va_list *app);
	static vncError_t Fsm_SecurityChallenged_2_SessionEstablished_X_ServerInitOk(va_list *app);
	static vncError_t Fsm_SecurityChallenged_2_Terminated_X_ServerInitOk(va_list *app);

	static vncError_t Fsm_Any_2_Terminated_X_Disconnect(va_list *app);
	static vncError_t Fsm_Any_2_Terminated_X_Disconnected(va_list *app);

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable : 4251 )
#endif

private:
	bool m_bValid;
	char* m_pPassword;
	char* m_pServerHost;
	bool m_bIncrement;
	vncNetPort_t m_nServerPort;
	vncNetworkType_t m_eType;
	vncNetIPVersion_t m_eIPVersion;
	vncNetFd_t m_nServerFD;
	vncClientState_t m_eState;
	vncObjectWrapper<vncNetTransport*> m_pVNCTransport;
	vncObjectWrapper<vncStateMachine*> m_pVNCStateMachine;
	vncObjectWrapper<vncBuffer*> m_pVNCBuffer;
	vncObjectWrapper<vncDisplay*> m_pVNCDisplay;
	vncObjectWrapper<vncMutex*> m_pVNCMutex;
	
	struct {
		int minor;
		int major;
	} m_Version;
	
	vncRfbSecurity_t m_eSecurity;
	char m_aChallengResponse[16];
	
	struct{
		vncRfbPixelFormat_t PixelFormat;
		bool bSharedFlag;
	} m_Client;
	
	struct{
		uint16_t nWidth;
		uint16_t nHeight;
		vncRfbPixelFormat_t PixelFormat;
		uint32_t nNameLength;
		uint8_t* pName;
	} m_Server;
	
	void* m_pParentWindow;
	
	const vncClientCallback* m_pcCallback;

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
};

#endif /* OVNC_CLIENT_H */
