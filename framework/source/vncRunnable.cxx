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
#include "openvnc/vncRunnable.h"

vncRunnable::vncRunnable(ovcn_runnable_func_run runFunc)
:vncObject()
{
	m_bImportant = false;
	m_bRunning = false;
	m_bStarted = false;
	m_bInitialized = false;
	m_RunFunc = runFunc;


	m_pVNCSemaphore = vncSemaphore::New();
	m_pObjectsMutex = vncMutex::New(false);
	
	m_bInitialized = true;
}


vncRunnable::~vncRunnable()
{
	Stop();
	vncObjectSafeFree(m_pVNCSemaphore);
	vncObjectSafeFree(m_pVNCThread);
	
	m_pObjectsMutex->Lock();
	m_Objects.clear();
	m_pObjectsMutex->UnLock();

	vncObjectSafeFree(m_pObjectsMutex);
}

bool vncRunnable::IsValid()
{
	return (m_pVNCSemaphore && m_RunFunc);
}

vncError_t vncRunnable::Start()
{
	if(IsValid()){
		int ret = -1;
		if(IsRunning()) return vncError_InvalidState;

		// call run() function in new thread
		if(!m_pVNCThread && !(m_pVNCThread = vncThread::New(m_RunFunc, this))){
			OVNC_DEBUG_ERROR("Failed to start new thread");
			return vncError_ApplicationError;
		}
		
		// Do not set "running" to true here
		// Problem: When you try to stop the thread before it start
		// Will be done by "TSK_RUNNABLE_RUN_BEGIN" which is called into the thread
		//self->running = tsk_true;
		m_bStarted = true;
		return vncError_Ok;
	}
	else{
		OVNC_DEBUG_ERROR("Invalid state");
		return vncError_InvalidState;
	}
}

vncError_t vncRunnable::Stop()
{
	vncError_t retCode = vncError_Ok;

	if(IsValid()){
		if(!m_bInitialized) {
			if(!IsRunning()){
				/* already deinitialized */
				return vncError_Ok;
			}
			else{
				/* should never happen */
				OVNC_DEBUG_ERROR("Not initialized");
				return vncError_InvalidState;
			}
		}
		else if(!IsRunning()) {

			if(IsStarted()){
				unsigned count = 0;
				/* Thread is started but not running ==> Give it time.*/
				while(++count <= 5){
					vncThread::Sleep(count * 200);
					if(IsRunning()){
						goto stop;
					}
				}
			}
			else{
				return vncError_Ok; /* already stopped */
			} 
			return vncError_ApplicationError;
		}

stop:
		m_bRunning = false;
		m_pVNCSemaphore->Increment();
		
		if((retCode = m_pVNCThread->Join()) != vncError_Ok){
			m_bRunning = true;
			OVNC_DEBUG_ERROR("Failed to join the thread");
			return retCode;
		}

		m_bStarted = false;
	}
	return retCode;
}

vncError_t vncRunnable::Enqueue(vncObjectWrapper<vncObject*> pVNCObject)
{
	if(!IsInitialized()){
		OVNC_DEBUG_ERROR("Not initialized yet");
		return vncError_InvalidState;
	}
	m_pObjectsMutex->Lock();
	m_Objects.push_back(pVNCObject);
	m_pObjectsMutex->UnLock();
	return m_pVNCSemaphore->Increment();
}