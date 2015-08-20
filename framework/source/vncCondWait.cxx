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
#include "openvnc/vncCondWait.h"
#include "openvnc/vncMutex.h"
#include "openvnc/vncUtils.h"
#include "openvnc/vncDebug.h"


#if OVNC_UNDER_WINDOWS
#	include <windows.h>
#	include "openvnc/vncErrno.h"
#	define CONDWAIT_S void
	typedef HANDLE	CONDWAIT_T;
#	define TIMED_OUT	WAIT_TIMEOUT
#else
#	include <sys/time.h>
#	include <pthread.h>
#	define CONDWAIT_S pthread_cond_t
	typedef CONDWAIT_S* CONDWAIT_T;
#	define TIMED_OUT	ETIMEDOUT
#endif

#if defined(__GNUC__) || defined (__SYMBIAN32__)
#	include <errno.h>
#endif

typedef struct ovnc_condwait_s
{
	CONDWAIT_T pcond; /**< Pthread handle pointing to the internal condwait object. */
#if !OVNC_UNDER_WINDOWS
	vncObjectWrapper<vncMutex*> pVNCMutex;  /**< Locker. */
#endif
}
ovnc_condwait_t;

vncCondWait::vncCondWait()
:vncObject()
{
	m_pHandle = NULL;
	ovnc_condwait_t *condwait = (ovnc_condwait_t *)vncUtils::Calloc(1, sizeof(ovnc_condwait_t));
	
	if(condwait)
	{
#if OVNC_UNDER_WINDOWS
		condwait->pcond = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(!condwait->pcond)
		{
			OVNC_FREE(condwait);
		}
#else
		condwait->pcond = (CONDWAIT_T)vncUtils::Calloc(1, sizeof(CONDWAIT_S));
		if(pthread_cond_init(condwait->pcond, 0))
		{
			OVNC_DEBUG_ERROR("Failed to initialize the new conwait.");
		}

		if(!(condwait->mutex = vncMutex::New()))
		{
			pthread_cond_destroy(condwait->pcond);

			OVNC_FREE(condwait);
			OVNC_DEBUG_ERROR("Failed to initialize the internal mutex.");
		}
#endif
	}

	if(!condwait){
		OVNC_DEBUG_ERROR("Failed to create new conwait.");
	}

	m_pHandle = condwait, condwait = NULL;
}


vncCondWait::~vncCondWait()
{
	ovnc_condwait_t *condwait = (ovnc_condwait_t*)m_pHandle;
	
	if(condwait){
#if OVNC_UNDER_WINDOWS
		CloseHandle(condwait->pcond);
#else
		vncObjectSafeFree(condwait->pVNCMutex);
		pthread_cond_destroy(condwait->pcond);
		OVNC_FREE(condwait->pcond);
#endif
		vncUtils::Free((void**)&condwait);
	}
	m_pHandle = NULL;
}

bool vncCondWait::IsValid()
{
	return m_pHandle != NULL;
}

vncError_t vncCondWait::Wait()
{
	if(!IsValid()){
		OVNC_DEBUG_ERROR("Invalid condvar");
		return vncError_InvalidState;
	}

	int ret = EINVAL;
	ovnc_condwait_t *condwait = (ovnc_condwait_t*)m_pHandle;

#if OVNC_UNDER_WINDOWS
	if((ret = (WaitForSingleObject(condwait->pcond, INFINITE) == WAIT_FAILED) ? -1 : 0)){
		OVNC_DEBUG_ERROR("WaitForSingleObject function failed: %d", ret);
	}
#else
	if(condwait && condwait->pVNCMutex){
		condwait->pVNCMutex->Lock();
		if(ret = pthread_cond_wait(condwait->pcond, (pthread_mutex_t*)condwait->pVNCMutex->GetHandle())){
			OVNC_DEBUG_ERROR("pthread_cond_wait function failed: %d", ret);
		}
		condwait->pVNCMutex->UnLock();
	}
#endif
	return ret ? vncError_SystemError : vncError_Ok;
}

vncError_t vncCondWait::Wait(uint64_t nMilliseconds)
{
	if(!IsValid()){
		OVNC_DEBUG_ERROR("Invalid condvar");
		return vncError_InvalidState;
	}

#if OVNC_UNDER_WINDOWS
	DWORD ret = WAIT_FAILED;
#else
	int ret = EINVAL;
#endif
	ovnc_condwait_t *condwait = (ovnc_condwait_t*)m_pHandle;

#if OVNC_UNDER_WINDOWS
	if((ret = WaitForSingleObject(condwait->pcond, (DWORD)nMilliseconds)) != WAIT_OBJECT_0){
		if(ret == TIMED_OUT){
			/* OVNC_DEBUG_INFO("WaitForSingleObject function timedout: %d", ret); */
		}
		else{
			OVNC_DEBUG_ERROR("WaitForSingleObject function failed: %d", ret);
		}
		return ((ret == TIMED_OUT) ? vncError_Ok : vncError_SystemError);
	}
#else
	if(condwait && condwait->pVNCMutex){
		struct timespec   ts = {0, 0};
		struct timeval    tv = {0, 0};
		/*int rc =*/  ovnc_gettimeofday(&tv, 0);

		ts.tv_sec  = ( tv.tv_sec + ((long)nMilliseconds/1000) );
		ts.tv_nsec += ( (tv.tv_usec * 1000) + ((long)nMilliseconds % 1000 * 1000000) );
		if(ts.tv_nsec > 999999999) ts.tv_sec+=1, ts.tv_nsec = ts.tv_nsec % 1000000000;
		
		condwait->pVNCMutex->Lock();
		if(ret = pthread_cond_timedwait(condwait->pcond, (pthread_mutex_t*)condwait->pVNCMutex->GetHandle(), &ts)){
			if(ret == TIMED_OUT){
				/* OVNC_DEBUG_INFO("pthread_cond_timedwait function timedout: %d", ret); */
			}
			else{
				OVNC_DEBUG_ERROR("pthread_cond_timedwait function failed: %d", ret);
			}
		}

		condwait->pVNCMutex->UnLock();

		return ((ret == TIMED_OUT) ? vncError_Ok : vncError_SystemError);
	}
#endif

	return ret ? vncError_SystemError : vncError_Ok;
}

vncError_t vncCondWait::Signal()
{
	int ret = EINVAL;
	ovnc_condwait_t *condwait = (ovnc_condwait_t*)m_pHandle;

#if OVNC_UNDER_WINDOWS
	if(ret = ((SetEvent(condwait->pcond) && ResetEvent(condwait->pcond)) ? 0 : -1)){
		ret = GetLastError();
		OVNC_DEBUG_ERROR("SetEvent/ResetEvent function failed: %d", ret);
	}
#else
	if(condwait && condwait->pVNCMutex){
		condwait->pVNCMutex->Lock();
		if(ret = pthread_cond_signal(condwait->pcond)){
			OVNC_DEBUG_ERROR("pthread_cond_signal function failed: %d", ret);
		}
		condwait->pVNCMutex->UnLock();
	}
#endif
	return ret ? vncError_SystemError : vncError_Ok;
}

vncError_t vncCondWait::Broadcast()
{
	int ret = EINVAL;
	ovnc_condwait_t *condwait = (ovnc_condwait_t*)m_pHandle;

#if OVNC_UNDER_WINDOWS
	if(ret = ((SetEvent(condwait->pcond) && ResetEvent(condwait->pcond)) ? 0 : -1)){
		ret = GetLastError();
		OVNC_DEBUG_ERROR("SetEvent function failed: %d", ret);
	}
#else
	if(condwait && condwait->pVNCMutex){
		condwait->pVNCMutex->Lock();
		if(ret = pthread_cond_broadcast(condwait->pcond)){
			OVNC_DEBUG_ERROR("pthread_cond_broadcast function failed: %d", ret);
		}
		condwait->pVNCMutex->UnLock();
	}
#endif

	return ret ? vncError_SystemError : vncError_Ok;
}

vncObjectWrapper<vncCondWait*> vncCondWait::New()
{
	vncObjectWrapper<vncCondWait*> pVNCCondwait = new vncCondWait();
	if(pVNCCondwait && !pVNCCondwait->m_pHandle){
		vncObjectSafeFree(pVNCCondwait);
	}
	return pVNCCondwait;
}