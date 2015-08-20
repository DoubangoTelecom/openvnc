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
#ifndef OVNC_STATEMACHINE_H
#define OVNC_STATEMACHINE_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncUtils.h"
#include "openvnc/vncMutex.h"

#include <list>

#define OVNC_FSM_ONTERMINATED_F(This)				(ovnc_fsm_onterminated_f)(This)

#define ovnc_fsm_state_any -0xFFFF
#define ovnc_fsm_state_default -0xFFF0
#define ovnc_fsm_state_none -0xFF00
#define ovnc_fsm_state_final -0xF000

#define ovnc_fsm_action_any -0xFFFF

typedef int ovnc_fsm_state_id_t;
typedef int ovnc_fsm_action_id_t;
typedef bool (*ovnc_fsm_cond_f)(const void*, const void*);
typedef vncError_t (*ovnc_fsm_exec_f)(va_list *app);
typedef vncError_t (*ovnc_fsm_onterminated_f)(const void*);

#define OVNC_FSM_ADD(from, action, cond, to, exec, desc)\
	1,\
	(ovnc_fsm_state_id_t)from, \
	(ovnc_fsm_action_id_t)action, \
	(ovnc_fsm_cond_f)cond, \
	(ovnc_fsm_state_id_t)to, \
	(ovnc_fsm_exec_f)exec, \
	(const char*)desc
#define OVNC_FSM_ADD_ALWAYS(from, action, to, exec, desc) OVNC_FSM_ADD(from, action, &vncStateMachine::__ExecCondAlways__, to, exec, desc)
#define OVNC_FSM_ADD_NOTHING(from, action, cond, desc) OVNC_FSM_ADD(from, action, cond, from, &vncStateMachine::__ExecNothing__, desc)
#define OVNC_FSM_ADD_ALWAYS_NOTHING(from, desc)	OVNC_FSM_ADD(from, ovnc_fsm_action_any, &vncStateMachine::__ExecCondAlways__, from, ovnc_fsm_exec_nothing, desc)
#define OVNC_FSM_ADD_DEFAULT()
#define OVNC_FSM_ADD_NULL()\
	NULL

class vncStateMachineEntry : public vncObject
{
public:
	vncStateMachineEntry(ovnc_fsm_state_id_t from, 
		ovnc_fsm_action_id_t action, 
		ovnc_fsm_cond_f cond,
		ovnc_fsm_state_id_t to,
		ovnc_fsm_exec_f exec,
		const char* pDesc)
		:vncObject()
	{
		m_From = from;
		m_Action = action;
		m_Cond = cond;
		m_To = to;
		m_Exec = exec;
		m_pDesc = vncUtils::StringDup(pDesc);
	}

	virtual ~vncStateMachineEntry()
	{
		OVNC_SAFE_FREE(m_pDesc);
	}
	virtual OVNC_INLINE const char* GetObjectId() { return "vncStateMachineEntry"; }

	bool vncStateMachineEntry::operator <(const vncStateMachineEntry &other) const {
		// Put "Any" states at the bottom(Strong)
		if(m_From == ovnc_fsm_state_any){
			//return -20;
			return true;
		}
		else if(other.m_From == ovnc_fsm_state_any){
			//return +20;
			return false;
		}

		// Put "Any" actions at the bottom(Weak)
		if(m_Action == ovnc_fsm_action_any){
			//return -10;
			return true;
		}
		else if(m_Action == ovnc_fsm_action_any){
			//return +10;
			return false;
		}
	}

	OVNC_INLINE ovnc_fsm_state_id_t GetFrom(){ return m_From; }
	OVNC_INLINE ovnc_fsm_action_id_t GetAction(){ return m_Action; }
	OVNC_INLINE ovnc_fsm_cond_f GetCond(){ return m_Cond; }
	OVNC_INLINE ovnc_fsm_state_id_t GetTo(){ return m_To; }
	OVNC_INLINE ovnc_fsm_exec_f GetExec(){ return m_Exec; }
	OVNC_INLINE const char* GetDesc(){ return m_pDesc; }

private:
	ovnc_fsm_state_id_t m_From;
	ovnc_fsm_action_id_t m_Action;
	ovnc_fsm_cond_f m_Cond;
	ovnc_fsm_state_id_t m_To;
	ovnc_fsm_exec_f m_Exec;
	char* m_pDesc;
};

class vncStateMachine : public vncObject
{
protected:
	vncStateMachine(ovnc_fsm_state_id_t currState, ovnc_fsm_state_id_t termState);
public:
	virtual ~vncStateMachine();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncStateMachine"; }
	OVNC_INLINE void SetDebug(bool bDebug){ m_bDebug = bDebug; }
	OVNC_INLINE void SetTermIfExecFail(bool bTermIfExecFail){ m_bTermIfExecFail = bTermIfExecFail; }
	vncError_t SetTermCallback(ovnc_fsm_onterminated_f termCallbackFunc, const void* pcTermCallbackContext);
	vncError_t ExecuteAction(ovnc_fsm_action_id_t action, const void* pCondData1, const void* pCondData2, ...);

public:
	static vncError_t Set(vncObjectWrapper<vncStateMachine*>This, ...);
	static vncObjectWrapper<vncStateMachine*> New(ovnc_fsm_state_id_t currState, ovnc_fsm_state_id_t termState);
	static vncError_t __ExecNothing__(va_list *__app__){ return vncError_Ok; }
	static bool __ExecCondAlways__(const void* __A__, const void* __B__){ return true; }

private:
	vncObjectWrapper<vncMutex*> m_pEntriesMutex;
	std::list<vncObjectWrapper<vncStateMachineEntry*>> m_Entries;
	bool m_bDebug;
	bool m_bTermIfExecFail;
	ovnc_fsm_state_id_t m_StateCurr;
	ovnc_fsm_state_id_t m_StateTerm;
	ovnc_fsm_onterminated_f m_TermCallbackFunc;
	const void* m_pcTermCallbackContext;
};

#endif /* OVNC_STATEMACHINE_H */
