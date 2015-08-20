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
#ifndef OVNC_SERVER_H
#define OVNC_SERVER_H

#include "vncConfig.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncObject.h"
#include "openvnc/network/vncNetTransport.h"
#include "openvnc/vncMutex.h"
#include "openvnc/vncBuffer.h"
#include "openvnc/video/vncScreenGrabber.h"
#include "openvnc/rfb/vncRfbTypes.h"
#include "openvnc/rfb/vncRfbMsg.h"

#include <map>
#include <list>

typedef enum vncServerState_e
{
	vncServerState_None = 0,
}
vncServerState_t;

class vncServerPeer;

class OVNC_API vncServer : public vncRunnable
{
protected:
	vncServer(const char* pLocalHost = OVNC_NET_SOCKET_HOST_ANY, vncNetPort_t nLocalPort = OVNC_CONFIG_SERVER_PORT, vncNetworkType_t eType = vncNetworkType_TCP);

public:
	virtual ~vncServer();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncServer"; }
	virtual OVNC_INLINE bool IsValid(){ return m_bValid; }
	
	virtual vncError_t AddEncoding(vncRfbEncoding_t eEncoding, bool bBack = true);
	virtual vncError_t RemoveEncoding(vncRfbEncoding_t eEncoding);
	virtual vncError_t ClearEncoding(vncRfbEncoding_t eEncoding);
	virtual vncError_t SetPassword(const char* pPassword);
	virtual vncError_t Start();
	virtual vncError_t Stop();

	static vncObjectWrapper<vncServer*> New(const char* pLocalHost = OVNC_NET_SOCKET_HOST_ANY, vncNetPort_t nLocalPort = OVNC_CONFIG_SERVER_PORT, vncNetworkType_t eType = vncNetworkType_TCP);

private:
	vncError_t SendMessage(vncObjectWrapper<vncServerPeer*> pVNCPeer, vncObjectWrapper<vncMsg*>pVNCMsg);
	vncError_t SendProtocolVersion(vncObjectWrapper<vncServerPeer*> pVNCPeer);
	vncError_t SendSecurities(vncObjectWrapper<vncServerPeer*> pVNCPeer);
	vncError_t SendRandom16Bytes(vncObjectWrapper<vncServerPeer*> pVNCPeer);
	vncError_t SendSecurityResult(vncObjectWrapper<vncServerPeer*> pVNCPeer, vncRfbSecurityResult_t eResult);
	vncError_t SendFailureReason(vncObjectWrapper<vncServerPeer*> pVNCPeer, const char* pReason);
	vncError_t SendServerInit(vncObjectWrapper<vncServerPeer*> pVNCPeer);
	bool IsChallengeResponseOk(vncObjectWrapper<vncServerPeer*> pVNCPeer, const int8_t* pResponse, unsigned nResponseSize);
	vncError_t HandleClientMessage(vncObjectWrapper<vncServerPeer*> pVNCPeer, vncObjectWrapper<vncMsg*>pVNCMsg);
	static void* Run(void* _This);
	static int NetworkCallback(const vncNetTransportEvent* pcEvent);


#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable : 4251 )
#endif

private:
	bool m_bValid;
	vncServerState_t m_eState;
	char* m_pPassword;
	vncObjectWrapper<vncNetTransport*> m_pVNCTransport;
	std::map<vncNetFd_t, vncObjectWrapper<vncServerPeer*> > m_Peers;
	vncObjectWrapper<vncMutex*> m_pVNCPeersMutex;
	std::list<vncRfbEncoding_t> m_aEncodings;

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
};


#if !SWIG

typedef enum vncServerPeerState_e
{
	vncServerPeerState_None,
	vncServerPeerState_ProtocolVersionSent,
	vncServerPeerState_ProtocolVersionOk,
	vncServerPeerState_ProtocolVersionNOk,

	vncServerPeerState_SecuritiesSent,
	vncServerPeerState_SecuritiesOk,
	vncServerPeerState_SecuritiesNOk,

	vncServerPeerState_ChallengeSent,
	vncServerPeerState_ChallengeOk,
	vncServerPeerState_ChallengeNOk,

	vncServerPeerState_ClientInitReceived,
	vncServerPeerState_SessionEstablished,

	vncServerPeerState_InvalidData,
	vncServerPeerState_NetworkError,
}
vncServerPeerState_t;

class vncServerPeer : public vncObject
{
public:
	vncServerPeer(vncNetFd_t nFd)
	{
		m_nFd = nFd;
		m_eState = vncServerPeerState_None;
		m_pVNCBuffer = vncBuffer::New();
		m_Version.major = OVNC_CONFIG_VERSION_MAJOR;
		m_Version.minor = OVNC_CONFIG_VERSION_MINOR;
		m_eSecurity = vncRfbSecurity_None;
		m_bRamdom16BytesSet = false;
		m_bSharedFlag = false;
		memset(&m_PixelFormat, 0, sizeof(m_PixelFormat));
		m_eNegEncoding = vncRfbEncoding_Raw;
	}
	virtual ~vncServerPeer()
	{
		vncObjectSafeFree(m_pVNCBuffer);
		vncObjectSafeFree(m_pVNCScreenGrabber);
	}
	virtual OVNC_INLINE const char* GetObjectId() { return "vncServerPeer"; }
	virtual OVNC_INLINE void SetState(vncServerPeerState_t eState){ m_eState = eState; }
	virtual OVNC_INLINE vncServerPeerState_t GetState(){ return m_eState; }
	virtual OVNC_INLINE vncNetFd_t GetFd(){ return m_nFd; }
	virtual OVNC_INLINE vncObjectWrapper<vncBuffer*> GetBuffer(){ return m_pVNCBuffer; }
	virtual OVNC_INLINE void SetVersion(int nMajor, int nMinor){ m_Version.major = nMajor; m_Version.minor = nMinor; }
	virtual OVNC_INLINE void SetSecurity(vncRfbSecurity_t eSecurity){ m_eSecurity = eSecurity; }
	virtual OVNC_INLINE vncRfbSecurity_t GetSecurity(){ return m_eSecurity; }
	virtual OVNC_INLINE const int8_t* GetRandom16Bytes(){
		if(!m_bRamdom16BytesSet){
			for(int i = 0; i< 16; ++i){
				m_aRandom16Bytes[i] = rand() ^ rand();
			}
			m_bRamdom16BytesSet = true;
		}
		return m_aRandom16Bytes;
	}
	virtual OVNC_INLINE bool IsSharedFlag(){ return m_bSharedFlag; }
	virtual OVNC_INLINE void SetSharedFlag(bool bSharedFlag){ m_bSharedFlag = bSharedFlag; }
	virtual OVNC_INLINE const vncRfbPixelFormat_t* GetPixelFormat(){ return &m_PixelFormat; }
	virtual vncError_t SetPixelFormat(const vncRfbPixelFormat_t* pPixelFormat){
		if(!pPixelFormat){
			OVNC_DEBUG_ERROR("Invalid parameter");
			return vncError_InvalidParameter;
		}
		memcpy(&m_PixelFormat, pPixelFormat, sizeof(vncRfbPixelFormat_t));
		return vncError_Ok;
	}
	virtual vncError_t SetEncodings(const std::list<vncRfbEncoding_t>* pEncoding){
		if(!pEncoding){
			OVNC_DEBUG_ERROR("Invalid parameter");
			return vncError_InvalidParameter;
		}
		m_aEncodings.clear();
		std::list<vncRfbEncoding_t>::const_iterator iter = pEncoding->begin();
		while(iter != pEncoding->end()){
			m_aEncodings.push_back(*iter);
			++iter;
		}
		return vncError_Ok;
	}
	virtual OVNC_INLINE const std::list<vncRfbEncoding_t>* GetEncodings(){ return &m_aEncodings; }
	virtual OVNC_INLINE void SetNegEncoding(vncRfbEncoding_t eEncoding){ m_eNegEncoding = eEncoding; }
	virtual OVNC_INLINE vncRfbEncoding_t GetNegEncoding(){ return m_eNegEncoding; }
	virtual OVNC_INLINE vncError_t SetScreenGrabber(vncObjectWrapper<vncScreenGrabber*> pVNCScreenGrabber){
		if(!pVNCScreenGrabber){
			OVNC_DEBUG_ERROR("Invalid parameter");
			return vncError_InvalidParameter;
		}
		m_pVNCScreenGrabber = pVNCScreenGrabber;
		return vncError_Ok;
	}
	virtual OVNC_INLINE const vncObjectWrapper<vncScreenGrabber*> GetScreenGrabber(){
		return m_pVNCScreenGrabber; 
	}
	
private:
	vncServerPeerState_t m_eState;
	vncNetFd_t m_nFd;
	vncObjectWrapper<vncBuffer*> m_pVNCBuffer;
	struct{
		int major;
		int minor;
	} m_Version;
	vncRfbSecurity_t m_eSecurity;
	int8_t m_aRandom16Bytes[16];
	bool m_bRamdom16BytesSet;
	bool m_bSharedFlag;
	vncRfbPixelFormat_t m_PixelFormat;
	std::list<vncRfbEncoding_t> m_aEncodings;
	vncRfbEncoding_t m_eNegEncoding;
	vncObjectWrapper<vncScreenGrabber*> m_pVNCScreenGrabber;
};

#endif /* SWIG */


#endif /* OVNC_SERVER_H */
