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
#ifndef OVNC_RUNNABLE_H
#define OVNC_RUNNABLE_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncSemaphore.h"
#include "openvnc/vncMutex.h"
#include "openvnc/vncThread.h"

#include <list>

typedef void * (*ovcn_runnable_func_run)(void* This);

class OVNC_API vncRunnable : public vncObject
{
protected:
	vncRunnable(ovcn_runnable_func_run runFunc);

public:
	virtual ~vncRunnable();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncRunnable"; }
#if !SWIG
	virtual OVNC_INLINE bool IsRunning(){ return m_bRunning; }
	virtual OVNC_INLINE bool IsStarted(){ return m_bStarted; }
	virtual OVNC_INLINE void SetImportant(bool bImportant){ m_bInitialized = bImportant; }
	virtual OVNC_INLINE bool IsImportant(){ return m_bImportant; }
	virtual OVNC_INLINE bool IsInitialized(){ return m_bInitialized; }
	virtual OVNC_INLINE bool IsValid();
	virtual vncError_t Start();
	virtual vncError_t Stop();
	virtual OVNC_INLINE vncObjectWrapper<vncObject*> PopFirstObject()
	{
		vncObjectWrapper<vncObject*> object;
		m_pObjectsMutex->Lock();
		std::list<vncObjectWrapper<vncObject*>>::iterator iter = m_Objects.begin();
		if(iter != m_Objects.end()){
			object = *iter;
			m_Objects.erase(iter);
		}
		m_pObjectsMutex->UnLock();
		return object;
	}
	virtual vncError_t Enqueue(vncObjectWrapper<vncObject*> pVNCObject);
#endif

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable : 4251 )
#endif

private:
	vncObjectWrapper<vncThread*>m_pVNCThread;
	ovcn_runnable_func_run m_RunFunc;

protected:
	std::list<vncObjectWrapper<vncObject*> > m_Objects;
	vncObjectWrapper<vncMutex*> m_pObjectsMutex;
	vncObjectWrapper<vncSemaphore*>m_pVNCSemaphore;
	bool m_bRunning;
	bool m_bImportant;
	bool m_bStarted;
	bool m_bInitialized;

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
};
		
#define OVNC_RUNNABLE_RUN_BEGIN(This) \
	This->m_bRunning = true; \
	for(;;) { \
		This->m_pVNCSemaphore->Decrement(); \
		if(!This->IsRunning() && \
			(!This->IsImportant() || (This->IsImportant() && This->m_Objects.empty()))) \
			break;

#define OVNC_RUNNABLE_RUN_END(This) \
	} \
	This->m_bRunning = false;

#endif /* OVNC_RUNNABLE_H */