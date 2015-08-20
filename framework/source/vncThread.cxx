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
#include "openvnc/vncThread.h"
#include "openvnc/vncUtils.h"
#include "openvnc/vncDebug.h"

#if OVNC_UNDER_WINDOWS
#	include <windows.h>
#else
#	include <pthread.h>
#endif

vncThread::vncThread(void *(*start) (void *), void *arg)
:vncObject()
{
	m_pTid = NULL;

#if OVNC_UNDER_WINDOWS
	DWORD ThreadId;
	m_pTid = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start, arg, 0, &ThreadId);
#else
	if((m_pTid = tsk_calloc(1, sizeof(pthread_t)))){
		int ret;
		if((ret = pthread_create((pthread_t*)m_pTid, 0, start, arg))){
			OVNC_DEBUG_ERROR("pthread_create() failed with error code = %d", ret);
			return;
		}
	}
#endif
}

vncThread::~vncThread()
{
	if(IsValid()){
		Join();
	}
}

bool vncThread::IsValid()
{
	return m_pTid != NULL;
}

vncError_t vncThread::SetPriority(int32_t nPriority)
{
	return vncThread::SetPriority(m_pTid, nPriority);
}

vncError_t vncThread::Join()
{
	int ret;

	if(!IsValid()){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidState;
	}
	if(!m_pTid){
		OVNC_DEBUG_WARN("Cannot join NULL tid");
		return vncError_Ok;
	}
    
#if OVNC_UNDER_WINDOWS
	ret = (WaitForSingleObject(((HANDLE)m_pTid), INFINITE) == WAIT_FAILED) ? -1 : 0;
	if(ret == 0){
		CloseHandle(((HANDLE)m_pTid));
		m_pTid = NULL;
	}
#else
	if((ret = pthread_join(*((pthread_t*)m_pTid), 0)) == 0){
		vncUtils::Free(&m_pTid);
	}
#endif

	return ret ? vncError_SystemError : vncError_Ok;
}

vncError_t vncThread::SetPriority(void* pTid, int32_t nPriority)
{
	if(!pTid){
		OVNC_DEBUG_ERROR("Invalid thread identifier");
		return vncError_InvalidParameter;
	}
#if OVNC_UNDER_WINDOWS
	return SetThreadPriority(pTid, nPriority) ? vncError_Ok : vncError_SystemError;
#else
	struct sched_param sp;
	int ret;
    memset(&sp, 0, sizeof(struct sched_param));
    sp.sched_priority = nPriority;
    if ((ret = pthread_setschedparam(*((pthread_t*)pTid), SCHED_RR, &sp))) {
        OVNC_DEBUG_ERROR("Failed to change priority to %d with error code=%d", priority, ret);
        return vncError_SystemError;
    }
    return vncError_Ok;
#endif
}

vncError_t vncThread::SetPriority2(int32_t nPriority)
{
#if OVNC_UNDER_WINDOWS
	return vncThread::SetPriority((void*)GetCurrentThread(), nPriority);
#else
	pthread_t thread = pthread_self();
	return vncThread::SetPriority(&thread, priority);
#endif
}

vncError_t vncThread::Sleep(uint64_t nMilliseconds)
{
#if OVNC_UNDER_WINDOWS
	::Sleep((DWORD)nMilliseconds);
#else
	struct timespec interval; 

	interval.tv_sec = (long)(nMilliseconds/1000); 
	interval.tv_nsec = (long)(nMilliseconds%1000) * 1000000; 
	nanosleep(&interval, 0);
#endif
	return vncError_Ok;
}

vncObjectWrapper<vncThread*> vncThread::New(void *(*start) (void *), void *arg)
{
	vncObjectWrapper<vncThread*> pVNCThread = new vncThread(start, arg);
	if(pVNCThread && !pVNCThread->IsValid()){
		vncObjectSafeFree(pVNCThread);
	}
	return pVNCThread;
}