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
#include "openvnc/vncSemaphore.h"
#include "openvnc/vncUtils.h"
#include "openvnc/vncDebug.h"

// Apple claims that they fully support POSIX semaphore but ...
#if OVNC_UNDER_APPLE // Mac OSX/Darwin/Iphone/Ipod Touch
#	define OVNC_USE_NAMED_SEM	1
#else 
#	define OVNC_USE_NAMED_SEM	0
#endif

#if OVNC_UNDER_WINDOWS /* Windows XP/Vista/7/CE */

#	include <windows.h>
#	include "openvnc/vncErrno.h"
#	define SEMAPHORE_S void
	typedef HANDLE	SEMAPHORE_T;
#else /* All *nix */

#	include <pthread.h>
#	include <semaphore.h>
#	if OVNC_USE_NAMED_SEM
#	include <fcntl.h> /* O_CREAT */
#	include <sys/stat.h> /* S_IRUSR, S_IWUSR*/

		static int sem_count = 0;
		typedef struct named_sem_s
		{
			sem_t* sem;
			char* name;
		} named_sem_t;
#		define SEMAPHORE_S named_sem_t
#		define GET_SEM(PSEM) (((named_sem_t*)(PSEM))->sem)
#	else
#		define SEMAPHORE_S sem_t
#		define GET_SEM(PSEM) ((PSEM))
#	endif /* OVNC_USE_NAMED_SEM */
	typedef sem_t* SEMAPHORE_T;

#endif

#if defined(__GNUC__) || defined(__SYMBIAN32__)
#	include <errno.h>
#endif

vncSemaphore::vncSemaphore(int32_t nInitialValue/*=0*/)
:vncObject()
{
	m_pHandle = NULL;
	
#if OVNC_UNDER_WINDOWS
	m_pHandle = CreateSemaphore(NULL, nInitialValue, 0x7FFFFFFF, NULL);
#else
	m_pHandle = vncUtils::Calloc(1, sizeof(SEMAPHORE_S));
	
#if OVNC_USE_NAMED_SEM
	named_sem_t * nsem = (named_sem_t*)m_pHandle;
	tsk_sprintf(&(nsem->name), "/sem-%d", sem_count++);
	if((nsem->sem = sem_open(nsem->name, O_CREAT /*| O_EXCL*/, S_IRUSR | S_IWUSR, initial_val)) == SEM_FAILED)
	{
		OVNC_FREE(nsem->name);
#else
	if(sem_init((SEMAPHORE_T)m_pHandle, 0, nInitialValue))
	{
#endif
		OVNC_FREE(handle);
		OVNC_DEBUG_ERROR("Failed to initialize the new semaphore (errno=%d).", errno);
	}
#endif
	
	if(!m_pHandle){
		OVNC_DEBUG_ERROR("Failed to create new semaphore");
	}
}


vncSemaphore::~vncSemaphore()
{
	if(m_pHandle)
	{
#if OVNC_UNDER_WINDOWS
		CloseHandle((SEMAPHORE_T)m_pHandle);
		m_pHandle = NULL;
#else
#	if OVNC_USE_NAMED_SEM
		named_sem_t * nsem = ((named_sem_t*)m_pHandle);
		sem_close(nsem->sem);
		OVNC_FREE(nsem->name);
#else
		sem_destroy((SEMAPHORE_T)GET_SEM(m_pHandle));
#endif /* OVNC_USE_NAMED_SEM */
	vncUtils::Free(&m_pHandle);
#endif
	}
	else{
		OVNC_DEBUG_WARN("Cannot free an uninitialized semaphore object");
	}
}

bool vncSemaphore::IsValid()
{
	return m_pHandle != NULL;
}

vncError_t vncSemaphore::Increment()
{
	int ret = EINVAL;
	if(m_pHandle)
	{
#if OVNC_UNDER_WINDOWS
		if((ret = ReleaseSemaphore((SEMAPHORE_T)m_pHandle, 1L, 0L) ? 0 : -1))
#else
		if(ret = sem_post((SEMAPHORE_T)GET_SEM(m_pHandle)))
#endif
		{
			OVNC_DEBUG_ERROR("sem_post function failed: %d", ret);
		}
	}
	return ret ? vncError_SystemError : vncError_Ok;
}

vncError_t vncSemaphore::Decrement()
{
	int ret = EINVAL;
	if(m_pHandle)
	{
#if OVNC_UNDER_WINDOWS
		ret = (WaitForSingleObject((SEMAPHORE_T)m_pHandle, INFINITE) == WAIT_OBJECT_0 ? 0 : -1);
		if(ret)	OVNC_DEBUG_ERROR("sem_wait function failed: %d", ret);
#else
		do { 
			ret = sem_wait((SEMAPHORE_T)GET_SEM(m_pHandle)); 
		} 
		while ( errno == EINTR );
		if(ret)	OVNC_DEBUG_ERROR("sem_wait function failed: %d", errno);
#endif
	}

	return ret ? vncError_SystemError : vncError_Ok;
}

vncObjectWrapper<vncSemaphore*> vncSemaphore::New(int32_t nInitialValue/*=0*/)
{
	vncObjectWrapper<vncSemaphore*> pVNCSemaphore = new vncSemaphore(nInitialValue);
	if(pVNCSemaphore && !pVNCSemaphore->IsValid()){
		vncObjectSafeFree(pVNCSemaphore);
	}
	return pVNCSemaphore;
}