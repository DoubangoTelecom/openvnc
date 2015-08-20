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
#include "openvnc/vncMutex.h"
#include "openvnc/vncUtils.h"
#include "openvnc/vncDebug.h"

#if OVNC_UNDER_WINDOWS
#	include <windows.h>
#	include "openvnc/vncErrno.h"
	typedef HANDLE	MUTEX_T;
#	define MUTEX_S void
#	define OVNC_ERROR_NOT_OWNER ERROR_NOT_OWNER
#else
#	include <pthread.h>
#	define MUTEX_S pthread_mutex_t
	typedef MUTEX_S* MUTEX_T;
#	define OVNC_ERROR_NOT_OWNER EPERM
#endif

#if defined(__GNUC__) || defined(__SYMBIAN32__)
#	include <errno.h>
#endif

vncMutex::vncMutex(bool bRecursive/*=true*/)
:vncObject()
{
	m_pHandle = NULL;
	
#if OVNC_UNDER_WINDOWS
	m_pHandle = CreateMutex(NULL, FALSE, NULL);
#else
	int ret;
	pthread_mutexattr_t   mta;
	
	if((ret = pthread_mutexattr_init(&mta))){
		OVNC_DEBUG_ERROR("pthread_mutexattr_init failed with error code %d", ret);
		return tsk_null;
	}
	if(bRecursive && (ret = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE))){
		OVNC_DEBUG_ERROR("pthread_mutexattr_settype failed with error code %d", ret);
		pthread_mutexattr_destroy(&mta);
		return;
	}
	
	/* if we are here: all is ok */
	m_pHandle = tsk_calloc(1, sizeof(MUTEX_S));
	if(pthread_mutex_init((MUTEX_T)m_pHandle, &mta)){
		OVNC_FREE(m_pHandle);
	}
	pthread_mutexattr_destroy(&mta);
#endif
	
	if(!m_pHandle){
		OVNC_DEBUG_ERROR("Failed to create new mutex");
	}
}

vncMutex::~vncMutex()
{
	if(m_pHandle){
#if OVNC_UNDER_WINDOWS
		CloseHandle((MUTEX_T)m_pHandle);
		m_pHandle = NULL;
#else
		pthread_mutex_destroy((MUTEX_T)m_pHandle);
		vncUtils::Free(&m_pHandle);
#endif
	}
}

bool vncMutex::IsValid()
{
	return m_pHandle != NULL;
}

vncError_t vncMutex::Lock()
{
	if(!IsValid()){
		OVNC_DEBUG_ERROR("Invalid mutex");
		return vncError_InvalidState;
	}

	int ret = EINVAL;
#if OVNC_UNDER_WINDOWS
	if((ret = WaitForSingleObject((MUTEX_T)m_pHandle , INFINITE)) == WAIT_FAILED)
#else
	if(ret = pthread_mutex_lock((MUTEX_T)m_pHandle))
#endif
	{
		OVNC_DEBUG_ERROR("Failed to lock the mutex: %d", ret);
	}

	return ret ? vncError_SystemError : vncError_Ok;
}

vncError_t vncMutex::UnLock()
{
	if(!IsValid()){
		OVNC_DEBUG_ERROR("Invalid mutex");
		return vncError_InvalidState;
	}

	int ret = EINVAL;

#if OVNC_UNDER_WINDOWS
	if((ret = ReleaseMutex((MUTEX_T)m_pHandle) ? 0 : -1)){
		ret = GetLastError();
#else
	if(ret = pthread_mutex_unlock((MUTEX_T)m_pHandle))
	{
#endif
		if(ret == OVNC_ERROR_NOT_OWNER){
			OVNC_DEBUG_WARN("The calling thread does not own the mutex: %d", ret);
		}
		else{
			OVNC_DEBUG_ERROR("Failed to unlock the mutex: %d", ret);
		}
	}

	return ret ? vncError_SystemError : vncError_Ok;
}

vncObjectWrapper<vncMutex*> vncMutex::New(bool bRecursive/*=true*/)
{	
	vncObjectWrapper<vncMutex*> pVNCMutex = new vncMutex(bRecursive);
	if(pVNCMutex && !pVNCMutex->IsValid()){
		vncObjectSafeFree(pVNCMutex);
	}
	return pVNCMutex;

}