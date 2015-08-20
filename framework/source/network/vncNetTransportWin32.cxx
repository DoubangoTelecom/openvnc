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
#include "openvnc/network/vncNetTransportWin32.h"
#include "openvnc/vncUtils.h"


//
//	vncNetTransportSocketWin32
//

class vncNetTransportSocketWin32 : public vncNetTransportSocket
{
public:
	vncNetTransportSocketWin32(vncNetFd_t nFd, vncNetworkType_t eType, bool bOwner) 
		:vncNetTransportSocket(nFd, eType, bOwner)
	{
		m_pEvent = WSACreateEvent();
	}
	virtual ~vncNetTransportSocketWin32()
	{
		if(m_pEvent){
			WSACloseEvent(m_pEvent);
			m_pEvent = NULL;
		}
	}
	virtual OVNC_INLINE const char* GetObjectId() { return "vncNetTransportSocketWin32"; }
	virtual OVNC_INLINE WSAEVENT GetEvent(){ return m_pEvent; }

private:
	WSAEVENT m_pEvent;
};



//
//	vncNetTransportWin32
//

vncNetTransportWin32::vncNetTransportWin32(const char* pHost, vncNetPort_t nPort, vncNetworkType_t eType, const char* pDescription)
:vncNetTransport(pHost, nPort, eType, pDescription)
{
	m_nEventsCount = 0;
	memset(m_aEvents, 0, sizeof(m_aEvents));
	m_pEventsMutex = vncMutex::New(false);
}

vncNetTransportWin32::~vncNetTransportWin32()
{
	Stop();

	m_pEventsMutex->Lock();
	for(int i = 0; i< sizeof(m_aEvents)/sizeof(WSAEVENT); ++i){
		if(m_aEvents[i]){
			// Do not close the event => Tied to the socket and will be closed when the socket is destroyed
			m_aEvents[i] = 0;
		}
	}
	m_pEventsMutex->UnLock();

	vncObjectSafeFree(m_pEventsMutex);
}

vncError_t vncNetTransportWin32::Prepare()
{
	vncObjectWrapper<vncNetTransportSocketWin32*> pVNCMasterSocketWin32;
	int status;

	vncError_t ret = vncNetTransport::Prepare();
	if(ret == vncError_Ok){
		// start listening
		if((ret = vncNetUtils::SockFdListen(m_pVNCMaster->GetFd(), WSA_MAXIMUM_WAIT_EVENTS)) != vncError_Ok){
			OVNC_DEBUG_ERROR("Listen have failed");
			goto bail;
		}
	}
	// retrieve master socket
	pVNCMasterSocketWin32 = FindSocketWin32(m_pVNCMaster->GetFd());
	if(!pVNCMasterSocketWin32){
		OVNC_DEBUG_ERROR("Failed to find master socket");
		ret = vncError_ApplicationError;
		goto bail;
	}

	// set events on master
	if((status = WSAEventSelect(m_pVNCMaster->GetFd(), pVNCMasterSocketWin32->GetEvent(), vncNetUtils::IsDGram(pVNCMasterSocketWin32->GetType()) ? FD_READ : FD_ALL_EVENTS) == SOCKET_ERROR)){
		vncUtils_SystemPrintLastError("WSAEventSelect have failed");
		ret = vncError_ApplicationError;
		goto bail;
	}

bail:
	m_bPrepared = (ret == vncError_Ok);
	return ret;
}

vncError_t vncNetTransportWin32::UnPrepare()
{
	vncError_t ret;
	if((ret = vncNetTransport::UnPrepare()) == vncError_Ok){
		
	}
	return ret;
}

vncError_t vncNetTransportWin32::Stop()
{
	vncError_t ret;
	if((ret = vncNetTransport::Stop()) == vncError_Ok){
		if(m_nEventsCount > 0){
			WSASetEvent(m_aEvents[0]);
		}
		if(m_pMainThead){
			m_pMainThead->Join();
		}
	}
	return ret;
}

ovcn_runnable_func_run vncNetTransportWin32::GetMainthreadRunFnc()
{
	return vncNetTransportWin32::MainThread;
}

vncObjectWrapper<vncNetTransportSocketWin32*> vncNetTransportWin32::FindSocketWin32(vncNetFd_t nFd)
{
	vncObjectWrapper<vncNetTransportSocket*> pVNCSocket = FindSocket(nFd);
	return pVNCSocket ? dynamic_cast<vncNetTransportSocketWin32*>(*pVNCSocket) : NULL;
}

vncObjectWrapper<vncNetTransportSocketWin32*> vncNetTransportWin32::FindSocketWin32ByEvent(WSAEVENT pEvent)
{
	vncObjectWrapper<vncNetTransportSocketWin32*> pVNCSocketWin32;
	GetSocketsMutex()->Lock();
	std::map<vncNetFd_t, vncObjectWrapper<vncNetTransportSocket*> >::const_iterator iter = GetSockets()->begin();
	while(iter != GetSockets()->end()){
		if(dynamic_cast<vncNetTransportSocketWin32*>(*iter->second)->GetEvent() == pEvent){
			pVNCSocketWin32 = dynamic_cast<vncNetTransportSocketWin32*>(*iter->second);
			goto done;
		}
		++iter;
	}
done:
	GetSocketsMutex()->UnLock();
	return pVNCSocketWin32;
}

vncObjectWrapper<vncNetTransportSocket*> vncNetTransportWin32::CreateSocket(vncNetFd_t nFd, vncNetworkType_t eType, bool bOwner)
{
	vncObjectWrapper<vncNetTransportSocketWin32*> pVNCSocket = new vncNetTransportSocketWin32(nFd, eType, bOwner);
	if(pVNCSocket){
		return dynamic_cast<vncNetTransportSocket*>(*pVNCSocket);
	}
	return NULL;
}

vncError_t vncNetTransportWin32::AddSocket(vncNetFd_t nFd, vncNetworkType_t eType, bool bTakeOwnership, bool bClient)
{
	vncError_t ret = vncNetTransport::AddSocket(nFd, eType, bTakeOwnership, bClient);
	if(ret == vncError_Ok){
		vncObjectWrapper<vncNetTransportSocketWin32*> pVNCSocketWin32 = FindSocketWin32(nFd);
		if(pVNCSocketWin32){
			AddEvent(pVNCSocketWin32->GetEvent());
			if(WSAEventSelect(pVNCSocketWin32->GetFd(), pVNCSocketWin32->GetEvent(), FD_ALL_EVENTS) == SOCKET_ERROR){
				RemoveSocket(pVNCSocketWin32->GetFd());
				vncUtils_SystemPrintLastError("WSAEventSelect have failed");
				return vncError_SystemError;
			}
			// Signal (Master event) that new socket has been added
			if(m_nEventsCount > 0 && WSASetEvent(m_aEvents[0]) == TRUE){
				OVNC_DEBUG_INFO("New socket (%d) added to the network transport", pVNCSocketWin32->GetFd());
			}
		}
		else{
			OVNC_DEBUG_ERROR("Failed to find socket");
			ret = vncError_ApplicationError;
			goto bail;
		}
	}
bail:
	return ret;
}

vncError_t vncNetTransportWin32::RemoveSocket(vncNetFd_t nFd, bool bRaiseClosed)
{
	vncObjectWrapper<vncNetTransportSocketWin32*> pVNCSocket = FindSocketWin32(nFd);
	if(pVNCSocket){
		// Ignore "FD_READ"
		WSAEventSelect(pVNCSocket->GetFd(), pVNCSocket->GetEvent(), FD_CLOSE);
		// Remove the socket to avoid Raising events on closed socket
		RemoveEventAtIndex(FindEventIndex(pVNCSocket->GetEvent()));
		// Signal that the event array have changed
		if(m_nEventsCount > 0 && WSASetEvent(m_aEvents[0]/*Master*/)){
			OVNC_DEBUG_INFO("Old socket removed from the network transport.");
		}
	}
	return vncNetTransport::RemoveSocket(nFd, bRaiseClosed);
}

bool vncNetTransportWin32::IsValid()
{
	if(!vncNetTransport::IsValid()){
		return false;
	}

	return true;
}

void vncNetTransportWin32::AddEvent(const WSAEVENT pcEvent)
{
	m_pEventsMutex->Lock();
	if(pcEvent){
		if(m_nEventsCount < sizeof(m_aEvents)/sizeof(WSAEVENT)-1){
			m_aEvents[m_nEventsCount] = pcEvent;
			++m_nEventsCount;
		}
		else{
			OVNC_DEBUG_ERROR("Too many events");
		}
	}
	m_pEventsMutex->UnLock();
}

void vncNetTransportWin32::RemoveEventAtIndex(int nIndex)
{
	m_pEventsMutex->Lock();

	if(nIndex < (int)m_nEventsCount && nIndex >= 0){
		for(unsigned i = nIndex; i< m_nEventsCount-1; ++i){
			m_aEvents[i] = m_aEvents[i + 1];
		}
	}
	m_aEvents[m_nEventsCount - 1] = NULL;
	--m_nEventsCount;

	m_pEventsMutex->UnLock();
}

int vncNetTransportWin32::FindEventIndex(const WSAEVENT pcEvent)
{
	int index = -1;

	m_pEventsMutex->Lock();
	
	for(int i = 0; i< (int)m_nEventsCount; ++i){
		if(m_aEvents[i] == pcEvent){
			index = i;
			break;
		}
	}
	m_pEventsMutex->UnLock();

	return index;
}

void* vncNetTransportWin32::MainThread(void* _This)
{
	vncNetTransportWin32* This = (vncNetTransportWin32*)_This;

	DWORD nEvent;
	WSANETWORKEVENTS networkEvents;
	DWORD nFlags = 0;
	int nRet;

	struct sockaddr_storage remoteAddr = {0};
	WSAEVENT pActiveEvent;
	vncObjectWrapper<vncNetTransportSocketWin32*> pActiveSocket;
	int nIndex;

	OVNC_DEBUG_INFO("Starting [%s] server with IP {%s} on port {%d}...", This->GetDescription(), *This->m_pVNCMaster->GetIP(), This->m_pVNCMaster->GetPort());
	
	while(This->IsRunning() || This->IsStarted())
	{
		// Wait for multiple events
		if((nEvent = WSAWaitForMultipleEvents(This->m_nEventsCount, This->m_aEvents, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED){
			vncUtils_SystemPrintLastError("WSAWaitForMultipleEvents have failed.");
			goto bail;
		}

		if(!This->IsRunning() && !This->IsStarted()){
			goto bail;
		}
	
		// lock events
		This->m_pEventsMutex->Lock();

		// find active event and socket
		nIndex = (nEvent - WSA_WAIT_EVENT_0);
		if(!(pActiveEvent = (WSAEVENT)This->m_aEvents[nIndex])){
			OVNC_DEBUG_ERROR("Failed to find event at index %d", nIndex);
			goto done;
		}
		if(!(pActiveSocket = This->FindSocketWin32ByEvent(pActiveEvent))){
			OVNC_DEBUG_ERROR("Failed to find socket at index %d", nIndex);
			goto done;
		}

		// Get the network events flags
		if (WSAEnumNetworkEvents(pActiveSocket->GetImmutableFd(), pActiveEvent, &networkEvents) == SOCKET_ERROR){
			vncObjectWrapper<vncNetTransportEvent*>pVNCEvent = new vncNetTransportEvent(
				vncNetTransportEventType_Error,
				This->GetCallbackContext(),
				pActiveSocket->GetFd());

			This->Enqueue(dynamic_cast<vncObject*>(*pVNCEvent));

			This->m_pEventsMutex->UnLock();
			vncUtils_SystemPrintLastError("WSAEnumNetworkEvents Failed");
			goto bail;
		}



		/*================== FD_ACCEPT ==================*/
		if(networkEvents.lNetworkEvents & FD_ACCEPT)
		{
			vncNetFd_t nFd;
			
			OVNC_DEBUG_INFO("NETWORK EVENT FOR SERVER [%s] -- FD_ACCEPT", This->GetDescription());

			if(networkEvents.iErrorCode[FD_ACCEPT_BIT]){
				vncObjectWrapper<vncNetTransportEvent*>pVNCEvent = new vncNetTransportEvent(
					vncNetTransportEventType_Error,
					This->GetCallbackContext(),
					pActiveSocket->GetFd());
				
				This->Enqueue(dynamic_cast<vncObject*>(*pVNCEvent));

				vncUtils_SystemPrintLastError("ACCEPT FAILED");
				goto done;
			}
			
			// Accept the connection
			if((nFd = WSAAccept(pActiveSocket->GetFd(), NULL, NULL, AcceptCondFunc, (DWORD_PTR)This)) != INVALID_SOCKET){
				// Add the new fd to the server context
				This->AddSocket(nFd, This->m_pVNCMaster->GetType(), true, false);
				if(WSAEventSelect(nFd, This->m_aEvents[This->m_nEventsCount - 1], FD_ALL_EVENTS) == SOCKET_ERROR){
					This->RemoveSocket(nFd);
					vncUtils_SystemPrintLastError("WSAEventSelect have failed.");
					goto done;
				}
				vncObjectWrapper<vncNetTransportEvent*>pVNCEvent = new vncNetTransportEvent(
					vncNetTransportEventType_Accepted,
					This->GetCallbackContext(),
					nFd);
				
				This->Enqueue(dynamic_cast<vncObject*>(*pVNCEvent));
			}
			else{
				vncUtils_SystemPrintLastError("ACCEPT FAILED.");
				goto done;
			}
		}



		/*================== FD_CONNECT ==================*/
		if(networkEvents.lNetworkEvents & FD_CONNECT)
		{
			OVNC_DEBUG_INFO("NETWORK EVENT FOR SERVER [%s] -- FD_CONNECT", This->GetDescription());

			if(networkEvents.iErrorCode[FD_CONNECT_BIT]){
				vncObjectWrapper<vncNetTransportEvent*>pVNCEvent = new vncNetTransportEvent(
					vncNetTransportEventType_Error,
					This->GetCallbackContext(),
					pActiveSocket->GetFd());
				
				This->Enqueue(dynamic_cast<vncObject*>(*pVNCEvent));

				vncUtils_SystemPrintLastError("CONNECT FAILED.");
				goto done;
			}
			else{
				pActiveSocket->SetConnected(true);

				vncObjectWrapper<vncNetTransportEvent*>pVNCEvent = new vncNetTransportEvent(
					vncNetTransportEventType_Connected,
					This->GetCallbackContext(),
					pActiveSocket->GetFd());

				This->Enqueue(dynamic_cast<vncObject*>(*pVNCEvent));
			}
		}


		/*================== FD_READ ==================*/
		if(networkEvents.lNetworkEvents & FD_READ)
		{
			DWORD readCount = 0;
			WSABUF wsaBuffer;

			OVNC_DEBUG_INFO("NETWORK EVENT FOR SERVER [%s] -- FD_READ", This->GetDescription());

			// check whether the socket is paused or not
			if(pActiveSocket->IsPaused()){
				OVNC_DEBUG_INFO("Socket is paused");
				goto FD_READ_DONE;
			}
			
			if(networkEvents.iErrorCode[FD_READ_BIT]){
				vncObjectWrapper<vncNetTransportEvent*>pVNCEvent = new vncNetTransportEvent(
					vncNetTransportEventType_Error,
					This->GetCallbackContext(),
					pActiveSocket->GetFd());
				
				This->Enqueue(dynamic_cast<vncObject*>(*pVNCEvent));

				vncUtils_SystemPrintLastError("READ FAILED.");
				goto done;
			}

			// nRetrieve the amount of pending data
			if(ovnc_net_ioctlt(pActiveSocket->GetFd(), FIONREAD, &(wsaBuffer.len)) < 0){
				vncUtils_SystemPrintLastError("IOCTLT FAILED.");
				goto done;
			}

			if(!wsaBuffer.len){
				goto done;
			}

			// alloc data
			if(!(wsaBuffer.buf = (CHAR*)vncUtils::Calloc(wsaBuffer.len, sizeof(uint8_t)))){
				goto done;
			}

			// Receive the waiting data
			if(/*active_socket->tlshandle*/false){
				/*int isEncrypted;
				tsk_size_t len = wsaBuffer.len;
				if(!(nRet = tnet_tls_socket_recv(active_socket->tlshandle, &wsaBuffer.buf, &len, &isEncrypted))){
					if(isEncrypted){
						OVNC_FREE(wsaBuffer.buf);
						goto done;
					}
					wsaBuffer.len = len;
				}*/
			}
			else{
				if(vncNetUtils::IsStream(This->m_pVNCMaster->GetType())){
					nRet = WSARecv(pActiveSocket->GetFd(), &wsaBuffer, 1, &readCount, &nFlags, 0, 0);
				}
				else{
					int len = sizeof(remoteAddr);
					nRet = WSARecvFrom(pActiveSocket->GetFd(), &wsaBuffer, 1, &readCount, &nFlags, 
						(struct sockaddr*)&remoteAddr, &len, 0, 0);
				}
				if(readCount < wsaBuffer.len){
					wsaBuffer.len = readCount;
					/* wsaBuffer.buf = tsk_realloc(wsaBuffer.buf, readCount); */
				}
			}

			if(nRet){
				nRet = WSAGetLastError();
				if(nRet == WSAEWOULDBLOCK){
					OVNC_DEBUG_WARN("WSAEWOULDBLOCK error for READ SSESSION");
				}
				else if(nRet == WSAECONNRESET && vncNetUtils::IsDGram(This->m_pVNCMaster->GetType())){
					// For DGRAM ==> The sent packet gernerated "ICMP Destination/Port unreachable" result
					OVNC_FREE(wsaBuffer.buf);
					goto done; // ignore and nRetry.
				}
				else{
					OVNC_FREE(wsaBuffer.buf);

					This->RemoveSocket(pActiveSocket->GetFd());
					vncUtils_SystemPrintLastError("WSARecv have failed.");
					goto done;
				}
			}
			else
			{	
				vncObjectWrapper<vncNetTransportEvent*>pVNCEvent = new vncNetTransportEvent(
					vncNetTransportEventType_Data,
					This->GetCallbackContext(),
					pActiveSocket->GetFd());
				
				if(pVNCEvent->SetData((void**)&wsaBuffer.buf, wsaBuffer.len, true) == vncError_Ok){
					pVNCEvent->SetRemoteAddr(&remoteAddr);
					This->Enqueue(dynamic_cast<vncObject*>(*pVNCEvent));
				}
				OVNC_SAFE_FREE(wsaBuffer.buf);
			}
FD_READ_DONE:;
		}
		
		
		
		
		/*================== FD_WRITE ==================*/
		if(networkEvents.lNetworkEvents & FD_WRITE)
		{
			//OVNC_DEBUG_INFO("NETWORK EVENT FOR SERVER [%s] -- FD_WRITE", This->GetDescription());

			if(networkEvents.iErrorCode[FD_WRITE_BIT]){
				vncObjectWrapper<vncNetTransportEvent*>pVNCEvent = new vncNetTransportEvent(
					vncNetTransportEventType_Error,
					This->GetCallbackContext(),
					pActiveSocket->GetFd());

				This->Enqueue(dynamic_cast<vncObject*>(*pVNCEvent));
				
				vncUtils_SystemPrintLastError("WRITE FAILED.");
				goto done;
			}			
		}
		


		/*================== FD_CLOSE ==================*/
		if(networkEvents.lNetworkEvents & FD_CLOSE){
			OVNC_DEBUG_INFO("NETWORK EVENT FOR SERVER [%s] -- FD_CLOSE", This->GetDescription());
			
			vncObjectWrapper<vncNetTransportEvent*>pVNCEvent = new vncNetTransportEvent(
					vncNetTransportEventType_Closed,
					This->GetCallbackContext(),
					pActiveSocket->GetFd());

				This->Enqueue(dynamic_cast<vncObject*>(*pVNCEvent));
			
			This->RemoveSocket(pActiveSocket->GetFd(), false);
		}

		/*	http://msdn.microsoft.com/en-us/library/ms741690(VS.85).aspx

			The proper way to reset the state of an event object used with the WSAEventSelect function 
			is to pass the handle of the event object to the WSAEnumNetworkEvents function in the hEventObject parameter. 
			This will reset the event object and adjust the status of active FD events on the socket in an atomic fashion.
		*/
		/* WSAResetEvent(active_event); <== DO NOT USE (see above) */

done:
		// unlock events
		This->m_pEventsMutex->UnLock();

	} /* while(transport->running) */
	

bail:


	OVNC_DEBUG_INFO("Stopped [%s] server with IP {%s} on port {%d}...", This->GetDescription(), *This->m_pVNCMaster->GetIP(), This->m_pVNCMaster->GetPort());

	return NULL;
}

int CALLBACK vncNetTransportWin32::AcceptCondFunc(LPWSABUF lpCallerId, LPWSABUF lpCallerData, LPQOS lpSQos, LPQOS lpGQos, LPWSABUF lpCalleeId, LPWSABUF lpCalleeData, GROUP FAR *Group, DWORD CallbackData)
{
	vncNetTransportWin32 *This = (vncNetTransportWin32*)CallbackData;
	return This->m_nEventsCount < WSA_MAXIMUM_WAIT_EVENTS ? CF_ACCEPT : CF_REJECT;
}

vncObjectWrapper<vncNetTransportWin32*> vncNetTransportWin32::New(const char* pHost, vncNetPort_t nPort, vncNetworkType_t eType, const char* pDescription)
{
	vncObjectWrapper<vncNetTransportWin32*> pVNCTransport = new vncNetTransportWin32(pHost, nPort, eType, pDescription);
	if(pVNCTransport && ! pVNCTransport->IsValid()){
		vncObjectSafeFree(pVNCTransport);
	}
	return pVNCTransport;
}
