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
#ifndef OVNC_NET_TRANSPORT_H
#define OVNC_NET_TRANSPORT_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncRunnable.h"
#include "openvnc/vncMutex.h"
#include "openvnc/network/vncNetSocket.h"
#include "openvnc/network/vncNetUtils.h"
#include "openvnc/vncUtils.h"

#include <map>

class vncNetTransportEvent;
typedef int (*ovnc_nettransport_cb_f)(const vncNetTransportEvent* pcEvent);

class vncNetTransportSocket : public vncObject
{
public:
	vncNetTransportSocket(vncNetFd_t nFd, vncNetworkType_t eType, bool bOwner) :vncObject()
	{
		m_nFd = nFd;
		m_nImmutableFd = nFd;
		m_eType = eType;
		m_bOwner = bOwner;
		m_bConnected = false;
		m_bPaused = false;
	}
	virtual ~vncNetTransportSocket()
	{
		Close();
	}
	virtual OVNC_INLINE const char* GetObjectId() { return "vncNetTransportSocket"; }
	virtual OVNC_INLINE vncNetFd_t GetFd(){ return m_nFd; }
	virtual OVNC_INLINE vncNetFd_t GetImmutableFd(){ return m_nImmutableFd; }
	virtual OVNC_INLINE vncNetworkType_t GetType(){ return m_eType; }
	virtual OVNC_INLINE bool IsOwner(){ return m_bOwner; }
	virtual OVNC_INLINE bool IsPaused(){ return m_bPaused; }
	virtual OVNC_INLINE void SetPaused(bool bPaused){ m_bPaused = bPaused; }
	virtual OVNC_INLINE bool IsConnected(){ return m_bConnected; }
	virtual OVNC_INLINE void SetConnected(bool bConnected){ m_bConnected = bConnected; }
	virtual vncNetFd_t Close(){
		if(m_bOwner && m_nFd != OVNC_NET_INVALID_FD){
			return (vncNetUtils::CloseSocketFd(&m_nFd) == 0) ? vncError_Ok : vncError_NetworkError;
		}
		return vncError_InvalidState;
	}

private:
	vncNetFd_t m_nFd;
	vncNetFd_t m_nImmutableFd; // Always keep the same value even after Close()
	vncNetworkType_t m_eType;
	bool m_bOwner;
	bool m_bConnected;
	bool m_bPaused;
};

typedef enum vncNetTransportEventType_e
{
	vncNetTransportEventType_None,

	vncNetTransportEventType_Data,
	vncNetTransportEventType_Closed,
	vncNetTransportEventType_Error,
	vncNetTransportEventType_Connected,
	vncNetTransportEventType_Accepted
}
vncNetTransportEventType_t;

class vncNetTransportEvent : public vncObject
{
public:
	vncNetTransportEvent(vncNetTransportEventType_t eType, const void* pcContext, vncNetFd_t nFd)
		:vncObject()
	{
		m_eType = eType;
		m_pcContext = pcContext;
		m_nFd = nFd;

		m_pData = NULL;
		m_nDataSize = 0;
	}
	virtual ~vncNetTransportEvent()
	{
		OVNC_SAFE_FREE(m_pData);
	}
	virtual OVNC_INLINE const char* GetObjectId() { return "vncNetTransportEvent"; }
	virtual OVNC_INLINE vncNetTransportEventType_t GetType()const{ return m_eType; }
	virtual OVNC_INLINE const void* GetContext()const{ return m_pcContext; }
	virtual OVNC_INLINE vncNetFd_t GetFd()const{ return m_nFd; }
	virtual OVNC_INLINE const struct sockaddr_storage* GetRemoteAddr()const{ return &m_RemoteAddr; }
	virtual OVNC_INLINE const void* GetDataPtr()const{ return m_pData; }
	virtual OVNC_INLINE unsigned GetDataSize()const{ return m_nDataSize; }
	vncError_t SetData(void **ppData, unsigned nDataSize, bool bTakeOwnership)
	{
		if(!ppData || !*ppData || !nDataSize){
			OVNC_DEBUG_ERROR("Invalid parameter");
			return vncError_InvalidParameter;
		}

		if(bTakeOwnership){
			OVNC_SAFE_FREE(m_pData);
			m_pData = *ppData, *ppData = NULL;
			m_nDataSize = nDataSize;
		}
		else{
			if(!(m_pData = vncUtils::Realloc(m_pData, nDataSize))){
				OVNC_DEBUG_ERROR("Realloc(%u) failed", nDataSize);
				m_nDataSize = 0;
				return vncError_ApplicationError;
			}
			memcpy(m_pData, *ppData, nDataSize);
			m_nDataSize = nDataSize;
		}
		return vncError_Ok;
	}
	void SetRemoteAddr(struct sockaddr_storage *pRemoteAddr)
	{
		if(pRemoteAddr){
			m_RemoteAddr = *pRemoteAddr;
		}
	}

private:
	vncNetTransportEventType_t m_eType;
	const void* m_pcContext;
	vncNetFd_t m_nFd;
	void *m_pData;
	unsigned m_nDataSize;
	struct sockaddr_storage m_RemoteAddr;
};

class vncNetTransport : public vncRunnable
{
protected:
	vncNetTransport(const char* pHost, vncNetPort_t nPort, vncNetworkType_t eType, const char* pDescription);

public:
	virtual ~vncNetTransport();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncNetTransport"; }
	virtual OVNC_INLINE const char* GetDescription() { return m_pDescription; }
	virtual bool IsValid();
	virtual vncError_t SetCallback(ovnc_nettransport_cb_f callbackFunc, const void* pcCallbackContext);
	virtual OVNC_INLINE const void* GetCallbackContext(){ return m_pcCallbackContext; }
	virtual OVNC_INLINE const std::map<vncNetFd_t, vncObjectWrapper<vncNetTransportSocket*> >* GetSockets(){ return &m_Sockets; }
	virtual OVNC_INLINE vncObjectWrapper<vncMutex*> GetSocketsMutex(){ return m_pSocketsMutex; }
	virtual vncError_t Start();
	virtual vncError_t Stop();
	virtual vncObjectWrapper<vncNetTransportSocket*> CreateSocket(vncNetFd_t nFd, vncNetworkType_t eType, bool bOwner)=0;
	virtual vncError_t AddSocket(vncNetFd_t nFd, vncNetworkType_t eType, bool bTakeOwnership, bool bClient);
	virtual vncError_t RemoveSocket(vncNetFd_t nFd, bool bRaiseClosed = true);
	virtual vncError_t Prepare();
	virtual vncError_t UnPrepare();
	virtual ovcn_runnable_func_run GetMainthreadRunFnc()=0;
	
	virtual vncNetFd_t ConnectTo(const char* pHost, vncNetPort_t nPort, vncNetworkType_t eType, vncNetIPVersion_t eIPVersion=vncNetIPVersion_IPv4and6);
	virtual vncObjectWrapper<vncNetTransportSocket*> FindSocket(vncNetFd_t nFd);

	static vncObjectWrapper<vncNetTransport*>New(const char* pHost, vncNetPort_t nPort, vncNetworkType_t eType, const char* pDescription);

private:
	static void* Run(void* _This);

private:
	std::map<vncNetFd_t, vncObjectWrapper<vncNetTransportSocket*> > m_Sockets;
	vncObjectWrapper<vncMutex*> m_pSocketsMutex;

protected:
	vncNetworkType_t m_eType;
	char* m_pLocalIP;
	char* m_pDescription;
	vncObjectWrapper<vncNetSocket*> m_pVNCMaster;
	vncObjectWrapper<vncThread*>m_pMainThead;
	bool m_bPrepared;
	ovnc_nettransport_cb_f m_CallbackFunc;
	const void* m_pcCallbackContext;
};

#endif /* OVNC_NET_TRANSPORT_H */
