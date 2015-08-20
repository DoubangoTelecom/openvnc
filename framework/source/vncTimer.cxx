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
#include "openvnc/vncTimer.h"
#include "openvnc/vncUtils.h"

#include <functional>
#include <algorithm>

struct PredTimerById: public std::binary_function< vncObjectWrapper<vncTimer*>, uint64_t, bool > {
  bool operator () ( const vncObjectWrapper<vncTimer*> pcTimer, const uint64_t &nTimerId ) const {
    return pcTimer->GetId() == nTimerId;
    }
};

//
//	vncTimer
//
vncTimer::vncTimer(uint64_t nTimeOut, ovnc_timer_callback_f callbackFnc, const void* pcArg)
:vncObject()
{
	static uint64_t g_nUniqueTimerId = 1;

	m_nId = g_nUniqueTimerId++;
	m_nTimeOut = nTimeOut;
	m_CallbackFnc = callbackFnc;
	m_pcArg = pcArg;

	m_bCanceled = false;
	m_nTimeOut += vncUtils::GetNow();
}

vncTimer::~vncTimer()
{

}



//
//	vncTimerMgr
//
vncTimerMgr::vncTimerMgr()
:vncRunnable(&vncTimerMgr::Run)
{
	m_pVNCSemaphore = vncSemaphore::New();
	m_pVNCCondWait = vncCondWait::New();
	m_pVNCMutex = vncMutex::New(false);
}

vncTimerMgr::~vncTimerMgr()
{
	Stop();

	m_pVNCMutex->Lock();
	m_Timers.clear();
	m_pVNCMutex->UnLock();

	vncObjectSafeRelease(m_pVNCMainThread);
	vncObjectSafeRelease(m_pVNCCondWait);
	vncObjectSafeRelease(m_pVNCMutex);
	vncObjectSafeRelease(m_pVNCSemaphore);
}

vncError_t vncTimerMgr::Start()
{
	vncError_t ret = vncError_Ok;

	m_pVNCMutex->Lock();

	if(!vncRunnable::IsRunning() && !vncRunnable::IsStarted()){
		ret = vncRunnable::Start();
		if(ret != vncError_Ok){
			OVNC_DEBUG_ERROR("Failed to start Runnable");
			goto bail;
		}
	}
	else{
		OVNC_DEBUG_WARN("Timer manager already running");
	}

bail:
	m_pVNCMutex->UnLock();

	return ret;
}

vncError_t vncTimerMgr::Stop()
{
	vncError_t ret = vncError_InvalidState;
	// all functions called below are thread-safe ==> do not lock
	// "mainthread" uses manager->mutex and runs in a separate thread ==> deadlock

	if(vncRunnable::IsRunning()){
		ret = vncRunnable::Stop();
		if(ret != vncError_Ok){
			OVNC_DEBUG_ERROR("Failed to stop Runnable");
			goto bail;
		}
		m_pVNCSemaphore->Increment();
		m_pVNCCondWait->Signal();
		ret = m_pVNCMainThread->Join();
		goto bail;
	}
	else{
		ret = vncError_Ok;
		OVNC_DEBUG_INFO("Already stopped");
		goto bail;
	}

bail:
	return ret;
}

vncError_t vncTimerMgr::Schedule(uint64_t nTimeout, ovnc_timer_callback_f callbackFnc, const void *pcArg, uint64_t &nTimerId)
{
	nTimerId = OVNC_INVALID_TIMER_ID;

	if(vncRunnable::IsRunning() || vncRunnable::IsStarted()){
		vncObjectWrapper<vncTimer*>pVNCTimer = new vncTimer(nTimeout, callbackFnc, pcArg);
		if(pVNCTimer){
			nTimerId = pVNCTimer->GetId();

			m_pVNCMutex->Lock();
			m_Timers.push_back(pVNCTimer);
			m_Timers.sort();
			m_pVNCMutex->UnLock();

			m_pVNCCondWait->Signal();
			m_pVNCSemaphore->Increment();
			return vncError_Ok;
		}
		else{
			OVNC_DEBUG_ERROR("Failed to create Timer");
			return vncError_ApplicationError;
		}
	}
	else{
		OVNC_DEBUG_ERROR("Invalid state");
		return vncError_InvalidState;
	}
}

vncError_t vncTimerMgr::Cancel(uint64_t nTimerId)
{
	// Check validity
	if(!OVNC_TIMER_ID_IS_VALID(nTimerId) || m_Timers.empty()){ /* Very common. */
		return vncError_Ok;
	}

	if(vncRunnable::IsRunning()){
		m_pVNCMutex->Lock();
		std::list<vncObjectWrapper<vncTimer*> >::iterator iter = std::find_if(m_Timers.begin(), m_Timers.end(), std::bind2nd(PredTimerById(), nTimerId));
		if(iter != m_Timers.end()){
			(*iter)->SetCanceled(true);
			if(iter == m_Timers.begin()){
				/* The timer we are waiting on ? ==> remove it now. */
				m_pVNCCondWait->Signal();
			}
		}
		m_pVNCMutex->UnLock();
		return vncError_Ok;
	}
	else{
		OVNC_DEBUG_ERROR("Invalid state");
		return vncError_InvalidState;
	}
}

void* vncTimerMgr::MainThread(void *pArg)
{
	vncError_t ret;
	vncObjectWrapper<vncTimer*> pVNCTimer;
	uint64_t nNow;
	vncTimerMgr* This = dynamic_cast<vncTimerMgr*>((vncTimerMgr*)pArg);

	OVNC_DEBUG_INFO("TIMER MANAGER -- START");

	while(This->IsRunning()){
		This->m_pVNCSemaphore->Decrement();

peek_first:
		if(!This->IsRunning()){
			break;
		}
		
		This->m_pVNCMutex->Lock();
		pVNCTimer = This->m_Timers.empty() ? NULL : *This->m_Timers.begin();
		This->m_pVNCMutex->UnLock();

		if(pVNCTimer && !pVNCTimer->IsCanceled()) {
			nNow = vncUtils::GetNow();
			if(nNow >= pVNCTimer->GetTimeOut()){
				//TSK_DEBUG_INFO("Timer raise %llu", timer->id);

				This->m_pVNCMutex->Lock();
				This->Enqueue(dynamic_cast<vncObject*>(*pVNCTimer));
				This->m_Timers.remove_if(std::bind2nd(PredTimerById(), pVNCTimer->GetId()));
				This->m_pVNCMutex->UnLock();
			}
			else{
				if((ret = This->m_pVNCCondWait->Wait((pVNCTimer->GetTimeOut() - nNow))) != vncError_Ok){
					OVNC_DEBUG_ERROR("CONWAIT for timer manager failed");
					break;
				}
				else{
					goto peek_first;
				}
			}
		}
		else if(pVNCTimer){
			This->m_pVNCMutex->Lock();
			/* TSK_DEBUG_INFO("Timer canceled %llu", curr->id); */
			This->m_Timers.remove_if(std::bind2nd(PredTimerById(), pVNCTimer->GetId()));
			This->m_pVNCMutex->UnLock();
		}
	} /* while() */
	
	OVNC_DEBUG_INFO("TIMER MANAGER -- STOP");

	return NULL;
}

void* vncTimerMgr::Run(void *pArg)
{
	vncObjectWrapper<vncObject*> curr;
	vncTimerMgr* This = dynamic_cast<vncTimerMgr*>((vncTimerMgr*)pArg);

	This->m_bRunning = true; // VERY IMPORTANT --> needed by the main thread

	/* create main thread */
	if(!(This->m_pVNCMainThread = vncThread::New(vncTimerMgr::MainThread, This))){
		OVNC_DEBUG_FATAL("Failed to create mainthread\n");
		return NULL;
	}

	OVNC_DEBUG_INFO("Timer manager run()::enter");

	OVNC_RUNNABLE_RUN_BEGIN(This);

	if((curr = This->PopFirstObject())){
		vncObjectWrapper<vncTimer*> pVNCTimer = dynamic_cast<vncTimer*>(*curr);
		if(pVNCTimer && pVNCTimer->GetCallback()){
			pVNCTimer->GetCallback()(pVNCTimer->GetArg(), pVNCTimer->GetId());
		}
#if I_WANT_MY_MEMORY_RIGHT_NOW
		vncObjectSafeRelease(curr);
		vncObjectSafeRelease(pVNCTimer);
#endif
	}

	OVNC_RUNNABLE_RUN_END(This);

	OVNC_DEBUG_INFO("Timer manager run()::exit");

	return NULL;
}
