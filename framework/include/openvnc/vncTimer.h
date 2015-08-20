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
#ifndef OVNC_TIMER_H
#define OVNC_TIMER_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncRunnable.h"
#include "openvnc/vncCondWait.h"
#include "openvnc/vncMutex.h"
#include "openvnc/vncSemaphore.h"

#include <list>

#define OVNC_INVALID_TIMER_ID 0
#define OVNC_TIMER_ID_IS_VALID(id) ((id) != OVNC_INVALID_TIMER_ID)
typedef int (*ovnc_timer_callback_f)(const void* pcArg, uint64_t nTimerId);

class vncTimer : public vncObject
{
public:
	vncTimer(uint64_t nTimeOut, ovnc_timer_callback_f callbackFnc, const void* pcArg);
	virtual ~vncTimer();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncTimer"; }
	OVNC_INLINE uint64_t GetId()const{ return m_nId; }
	OVNC_INLINE void SetCanceled(bool bCanceled){ m_bCanceled = bCanceled; }
	OVNC_INLINE bool IsCanceled(){ return m_bCanceled; }
	OVNC_INLINE const void* GetArg(){ return m_pcArg; }
	OVNC_INLINE ovnc_timer_callback_f GetCallback(){ return m_CallbackFnc; }
	OVNC_INLINE uint64_t GetTimeOut(){ return m_nTimeOut; }
	OVNC_INLINE bool operator <(const vncTimer &other) const{
		return (m_nTimeOut < other.m_nTimeOut);
	}
	OVNC_INLINE bool operator >(const vncTimer &other) const{
		return (m_nTimeOut > other.m_nTimeOut);
	}

private:
	uint64_t m_nId;
	const void* m_pcArg;
	uint64_t m_nTimeOut;
	ovnc_timer_callback_f m_CallbackFnc;
	bool m_bCanceled;
};

class vncTimerMgr : public vncRunnable
{
protected:
	vncTimerMgr();

public:
	virtual ~vncTimerMgr();
	virtual vncError_t Start();
	virtual vncError_t Stop();
	vncError_t Schedule(uint64_t nTimeout, ovnc_timer_callback_f callbackFnc, const void *pcArg, uint64_t &nTimerId);
	vncError_t Cancel(uint64_t nTimerId);

private:
	static void* MainThread(void *pArg);
	static void* Run(void *pArg);

private:
	vncObjectWrapper<vncThread*> m_pVNCMainThread;
	vncObjectWrapper<vncCondWait*> m_pVNCCondWait;
	vncObjectWrapper<vncMutex*> m_pVNCMutex;
	vncObjectWrapper<vncSemaphore*> m_pVNCSemaphore;
	std::list<vncObjectWrapper<vncTimer*> > m_Timers;
};

#endif /* OVNC_TIMER_H */
