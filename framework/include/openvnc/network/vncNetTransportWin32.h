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
#ifndef OVNC_NET_TRANSPORT_WIN32_H
#define OVNC_NET_TRANSPORT_WIN32_H

#include "vncConfig.h"
#include "openvnc/network/vncNetTransport.h"
#include "openvnc/vncMutex.h"

class vncNetTransportSocketWin32;

class vncNetTransportWin32 : public vncNetTransport
{
protected:
	vncNetTransportWin32(const char* pHost, vncNetPort_t nPort, vncNetworkType_t eType, const char* pDescription);

public:
	virtual ~vncNetTransportWin32();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncNetTransportWin32"; }
	virtual bool IsValid();

	// Overrides from 'vncNetTransport'
	virtual vncError_t Prepare();
	virtual vncError_t UnPrepare();
	virtual vncError_t Stop();
	virtual ovcn_runnable_func_run GetMainthreadRunFnc();
	virtual vncObjectWrapper<vncNetTransportSocketWin32*> FindSocketWin32(vncNetFd_t nFd);
	virtual vncObjectWrapper<vncNetTransportSocketWin32*> FindSocketWin32ByEvent(WSAEVENT pEvent);
	virtual vncObjectWrapper<vncNetTransportSocket*> CreateSocket(vncNetFd_t nFd, vncNetworkType_t eType, bool bOwner);
	virtual vncError_t AddSocket(vncNetFd_t nFd, vncNetworkType_t eType, bool bTakeOwnership, bool bClient);
	virtual vncError_t RemoveSocket(vncNetFd_t nFd, bool bRaiseClosed = true);

	static vncObjectWrapper<vncNetTransportWin32*>New(const char* pHost, vncNetPort_t nPort, vncNetworkType_t eType, const char* pDescription);

private:
	void AddEvent(const WSAEVENT pcEvent);
	void RemoveEventAtIndex(int nIndex);
	int FindEventIndex(const WSAEVENT pcEvent);
	static void* MainThread(void* _This);
	static int CALLBACK AcceptCondFunc(LPWSABUF lpCallerId, LPWSABUF lpCallerData, LPQOS lpSQos, LPQOS lpGQos, LPWSABUF lpCalleeId, LPWSABUF lpCalleeData, GROUP FAR *Group, DWORD CallbackData);

private:
	vncObjectWrapper<vncMutex*> m_pEventsMutex;
	unsigned m_nEventsCount;
	WSAEVENT m_aEvents[WSA_MAXIMUM_WAIT_EVENTS];
};

#endif /* OVNC_NET_TRANSPORT_WIN32_H */
