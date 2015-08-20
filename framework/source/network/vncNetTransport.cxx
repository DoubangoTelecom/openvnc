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
#include "openvnc/network/vncNetTransport.h"
#include "openvnc/network/vncNetTransportWin32.h"
#include "openvnc/network/vncNetTransportLinux.h"
#include "openvnc/vncUtils.h"


vncNetTransport::vncNetTransport(const char* pHost, vncNetPort_t nPort, vncNetworkType_t eType, const char* pDescription)
:vncRunnable(vncNetTransport::Run)
{
	m_CallbackFunc = NULL;
	m_pcCallbackContext = NULL;
	m_pDescription = NULL;
	m_bPrepared = false;
	m_eType = eType;
	m_pLocalIP = NULL;
	m_pVNCMaster = vncNetSocket::New(pHost, nPort, eType);
	m_pSocketsMutex = vncMutex::New(false);
	
	if(m_pVNCMaster){
		if(m_eType == vncNetIPVersion_IPv4and6){
			m_pLocalIP = vncUtils::StringDup(pHost);// FQDN
		}
		else{
			m_pLocalIP = vncUtils::StringDup(*m_pVNCMaster->GetIP());
		}
		m_pDescription = vncUtils::StringDup(pDescription);
	}
}

vncNetTransport::~vncNetTransport()
{
	Stop();

	vncObjectSafeFree(m_pVNCMaster);
	OVNC_FREE(m_pLocalIP);
	OVNC_FREE(m_pDescription);

	m_pSocketsMutex->Lock();
	m_Sockets.clear();
	m_pSocketsMutex->UnLock();

	vncObjectSafeFree(m_pSocketsMutex);
	vncObjectSafeFree(m_pMainThead);
}

bool vncNetTransport::IsValid()
{
	return (m_pVNCMaster) && (m_pSocketsMutex);
}

vncError_t vncNetTransport::SetCallback(ovnc_nettransport_cb_f callbackFunc, const void* pcCallbackContext)
{
	m_CallbackFunc = callbackFunc;
	m_pcCallbackContext = pcCallbackContext;
	return vncError_Ok;
}

vncError_t vncNetTransport::Start()
{
	if(!IsValid()){
		OVNC_DEBUG_ERROR("Invalid state");
		return vncError_InvalidState;
	}
	vncError_t ret;

	// prepare transport
	if((ret = Prepare()) != vncError_Ok){
		OVNC_DEBUG_ERROR("Failed to prepare transport");
		goto bail;
	}

	// start transport
	if((ret = vncRunnable::Start()) != vncError_Ok){
		OVNC_DEBUG_ERROR("Failed to start transport");
		goto bail;
	}

bail:
	return ret;
}

vncError_t vncNetTransport::Stop()
{
	vncError_t ret;

	// unprepare transport
	if((ret = UnPrepare()) != vncError_Ok){
		OVNC_DEBUG_ERROR("Failed to unprepare transport");
		goto bail;
	}

	// stop transport
	if((ret = vncRunnable::Stop()) != vncError_Ok){
		OVNC_DEBUG_ERROR("Failed to stop transport");
		goto bail;
	}

bail:
	return ret;
}

vncError_t vncNetTransport::AddSocket(vncNetFd_t nFd, vncNetworkType_t eType, bool bTakeOwnership, bool bClient)
{
	vncObjectWrapper<vncNetTransportSocket*> pVNCSocket = CreateSocket(nFd, eType, bTakeOwnership);
	if(!pVNCSocket){
		OVNC_DEBUG_ERROR("Failed to create socket");
		return vncError_ApplicationError;
	}
	m_Sockets.insert(std::pair<vncNetFd_t, vncObjectWrapper<vncNetTransportSocket*> >(pVNCSocket->GetFd(), pVNCSocket));
	return vncError_Ok;
}

vncError_t vncNetTransport::RemoveSocket(vncNetFd_t nFd, bool bRaiseClosed)
{
	m_pSocketsMutex->Lock();
	std::map<vncNetFd_t, vncObjectWrapper<vncNetTransportSocket*> >::iterator iter 
		= m_Sockets.find(nFd);
	if(iter != m_Sockets.end()){
		if(iter->second->Close() == vncError_Ok && bRaiseClosed){
			vncObjectWrapper<vncNetTransportEvent*>pVNCEvent = new vncNetTransportEvent(
				vncNetTransportEventType_Closed,
				GetCallbackContext(),
				nFd);
			Enqueue(dynamic_cast<vncObject*>(*pVNCEvent));
		}
		vncObjectSafeRelease(iter->second);
		m_Sockets.erase(iter);
	}
	m_pSocketsMutex->UnLock();
	return vncError_Success;
}

vncError_t vncNetTransport::Prepare()
{
	if(m_bPrepared){
		OVNC_DEBUG_ERROR("Transport already prepared");
		return vncError_InvalidState;
	}
	// add master (or override if already exist)
	if(m_pVNCMaster){
		vncError_t ret;
		if((ret = AddSocket(m_pVNCMaster->GetFd(), m_pVNCMaster->GetType(), true, false)) != vncError_Ok){
			OVNC_DEBUG_ERROR("Failed to add master to the transport's sockets");
			return ret;
		}
	}
	m_bPrepared = true;
	return vncError_Ok;
}

vncError_t vncNetTransport::UnPrepare()
{
	return vncError_Ok;
}

vncNetFd_t vncNetTransport::ConnectTo(const char* pHost, vncNetPort_t nPort, vncNetworkType_t eType, vncNetIPVersion_t eIPVersion/*=vncNetIPVersion_IPv4and6*/)
{
	vncNetFd_t nFd = OVNC_NET_INVALID_FD;

	if(!IsValid() || !m_pVNCMaster){
		OVNC_DEBUG_ERROR("Invalid state");
		return OVNC_NET_INVALID_FD;
	}

	struct sockaddr_storage to;
	int status = -1;
	vncError_t ret;

	if((vncNetUtils::IsStream(m_pVNCMaster->GetType()) && !vncNetUtils::IsStream(eType)) ||
		(vncNetUtils::IsDGram(m_pVNCMaster->GetType()) && !vncNetUtils::IsDGram(eType))){
		OVNC_DEBUG_ERROR("Master/destination types mismatch [%u/%u]", m_pVNCMaster->GetType(), eType);
		goto bail;
	}

	// Init destination sockaddr fields

	if((ret = vncNetUtils::SockAddrInit(pHost, nPort, eType, &to)) != vncError_Ok){
		OVNC_DEBUG_ERROR("Invalid HOST/PORT [%s/%u]", pHost, nPort);
		goto bail;
	}
	else if(eIPVersion == vncNetIPVersion_IPv4and6){
		// Update the type (unambiguously)
		if(to.ss_family == AF_INET6){
			eIPVersion = vncNetIPVersion_IPv6;;
		}
		else{
			eIPVersion = vncNetIPVersion_IPv4;
		}
	}
	
	//
	// STREAM ==> create new socket and connect it to the remote host.
	// DGRAM ==> connect the master to the remote host.
	//
	if(vncNetUtils::IsStream(eType)){		
		// Create client socket descriptor
		if((ret = vncNetUtils::SockFdInit(m_pLocalIP, OVNC_NET_SOCKET_PORT_ANY, eType, &nFd)) != vncError_Ok){
			OVNC_DEBUG_ERROR("Failed to create new sockfd");
			goto bail;
		}
		
		// Add the socket
		if((ret = AddSocket(nFd, eType, true, true)) != vncError_Ok){
			vncUtils_SystemPrintLastError("Failed to add new socket");
			
			vncNetUtils::CloseSocketFd(&nFd);
			goto bail;
		}
	}
	else{
		nFd = m_pVNCMaster->GetFd();
	}
	
	if((status = vncNetUtils::SockFdConnectTo(nFd, (const struct sockaddr_storage *)&to))){
		if(nFd != m_pVNCMaster->GetFd()){
			vncNetUtils::CloseSocketFd(&nFd);
		}
		goto bail;
	}
	else{
		//if(TNET_SOCKET_TYPE_IS_TLS(type)){
		//	transport->tls.have_tls = tsk_true;
		//	/*transport->connected = !*/tnet_tls_socket_connect((tnet_tls_socket_handle_t*)tnet_transport_get_tlshandle(handle, fd));
		//}
		//else{
		//	//transport->connected = tsk_true;
		//}
	}

bail:
	return nFd;
}

vncObjectWrapper<vncNetTransportSocket*> vncNetTransport::FindSocket(vncNetFd_t nFd)
{
	m_pSocketsMutex->Lock();
	std::map<vncNetFd_t, vncObjectWrapper<vncNetTransportSocket*> >::iterator iter 
		= m_Sockets.find(nFd);
	m_pSocketsMutex->UnLock();
	return iter == m_Sockets.end() ? NULL : iter->second;
}

void* vncNetTransport::Run(void* _This)
{
	vncNetTransport* This = (vncNetTransport*)_This;
	vncObjectWrapper<vncObject*> curr;
	
	OVNC_DEBUG_INFO("vncNetTransport::Run::Enter[%s]", This->m_pDescription);

	if(!(This->m_pMainThead = vncThread::New(This->GetMainthreadRunFnc(), This))){
		OVNC_DEBUG_ERROR("Failed to start main thread");
		return NULL;
	}

	OVNC_RUNNABLE_RUN_BEGIN(This);

	if((curr = This->PopFirstObject())){
		vncObjectWrapper<vncNetTransportEvent*> pVNCEvent = dynamic_cast<vncNetTransportEvent*>(*curr);
		if(pVNCEvent && This->m_CallbackFunc){
			This->m_CallbackFunc(*pVNCEvent);
		}

#if I_WANT_MY_MEMORY_RIGHT_NOW
		vncObjectSafeRelease(curr);
		vncObjectSafeRelease(pVNCEvent);
#endif
	}

	OVNC_RUNNABLE_RUN_END(This);

	OVNC_DEBUG_INFO("vncNetTransport::Run::Exit[%s]", This->m_pDescription);

	return NULL;
}


vncObjectWrapper<vncNetTransport*> vncNetTransport::New(const char* pHost, vncNetPort_t nPort, vncNetworkType_t eType, const char* pDescription)
{
	vncObjectWrapper<vncNetTransport*> pVNCTransport;
#if OVNC_UNDER_WINDOWS
	vncObjectWrapper<vncNetTransportWin32*> pVNCTransportWin32 = vncNetTransportWin32::New(pHost, nPort, eType, pDescription);
	if(pVNCTransportWin32){
		pVNCTransport = dynamic_cast<vncNetTransport*>(*pVNCTransportWin32);
	}
#else
	pVNCTransport = new vncNetTransportLinux(pHost, nPort, eType, pDescription);
#endif
	if(pVNCTransport && !pVNCTransport->IsValid()){
		vncObjectSafeFree(pVNCTransport);
	}
	return pVNCTransport;
}
