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
#include "openvnc/vncStateMachine.h"

vncStateMachine::vncStateMachine(ovnc_fsm_state_id_t currState, ovnc_fsm_state_id_t termState)
{
#if defined(DEBUG) || defined(_DEBUG)
	m_bDebug = true;
#else
	m_bDebug = false;
#endif
	m_bTermIfExecFail = false;
	m_pEntriesMutex = vncMutex::New();
	m_pcTermCallbackContext = NULL;
	m_TermCallbackFunc = NULL;

	m_StateCurr = currState;
	m_StateTerm = termState;
}

vncStateMachine::~vncStateMachine()
{
	m_pEntriesMutex->Lock();
	m_Entries.clear();
	m_pEntriesMutex->UnLock();

	vncObjectSafeFree(m_pEntriesMutex);
}

vncError_t vncStateMachine::SetTermCallback(ovnc_fsm_onterminated_f termCallbackFunc, const void* pcTermCallbackContext)
{
	m_TermCallbackFunc = termCallbackFunc;
	m_pcTermCallbackContext = pcTermCallbackContext;
	return vncError_Ok;
}

vncError_t vncStateMachine::Set(vncObjectWrapper<vncStateMachine*>This, ...)
{
	va_list args;
	int guard;
	
	if(!This){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	
	va_start(args, This);
	while((guard = va_arg(args, int)) == 1){
		ovnc_fsm_state_id_t from = va_arg(args, ovnc_fsm_state_id_t); 
		ovnc_fsm_action_id_t action = va_arg(args, ovnc_fsm_action_id_t);
		ovnc_fsm_cond_f cond = va_arg(args, ovnc_fsm_cond_f);
		ovnc_fsm_state_id_t to = va_arg(args, ovnc_fsm_state_id_t);
		ovnc_fsm_exec_f exec = va_arg(args, ovnc_fsm_exec_f);
		const char* pDesc = va_arg(args, const char*);
		vncObjectWrapper<vncStateMachineEntry*> pEntry = new vncStateMachineEntry(from, action, cond, to, exec, pDesc);
		if(pEntry){
			This->m_Entries.push_back(pEntry);
		}
	}
	va_end(args);
	
	This->m_Entries.sort();

	return vncError_Ok;
}

vncError_t vncStateMachine::ExecuteAction(ovnc_fsm_action_id_t action, const void* pCondData1, const void* pCondData2, ...)
{
	va_list ap;
	bool bFound = false;
	bool bTerminates = false; /* thread-safeness -> DO NOT REMOVE THIS VARIABLE */
	vncError_t retExec = vncError_Ok;
	std::list<vncObjectWrapper<vncStateMachineEntry*>>::iterator iter;
	
	if(m_StateCurr == m_StateTerm){
		OVNC_DEBUG_WARN("The FSM is in the final state.");
		return vncError_InvalidState;
	}
	
	// lock
	m_pEntriesMutex->Lock();
	
	va_start(ap, pCondData2);
	for(iter = m_Entries.begin(); iter != m_Entries.end(); ++iter)
	{
		if(((*iter)->GetFrom() != ovnc_fsm_state_any) && ((*iter)->GetFrom() != m_StateCurr)){
			continue;
		}

		if(((*iter)->GetAction() != ovnc_fsm_action_any) && ((*iter)->GetAction() != action)){
			continue;
		}
		
		// check condition
		if((*iter)->GetCond()(pCondData1, pCondData2)){
			// For debug information
			if(m_bDebug){
				OVNC_DEBUG_INFO("State machine: %s", (*iter)->GetDesc());
			}
			
			if((*iter)->GetTo() != ovnc_fsm_action_any){ /* Stay at the current state if dest. state is Any */
				m_StateCurr = (*iter)->GetTo();
			}
			
			if((*iter)->GetExec()){
				if((retExec = (*iter)->GetExec()(&ap)) != vncError_Ok){
					OVNC_DEBUG_INFO("State machine: Exec function failed.");
				}
			}
			else{ /* Nothing to execute */
				retExec = vncError_Ok;
			}
			
			bTerminates = (((retExec != vncError_Ok) && m_bTermIfExecFail) || (m_StateCurr == m_StateTerm));
			bFound = true;
			break;
		}
	}
	va_end(ap);
	
	// unlock
	m_pEntriesMutex->UnLock();

	// Only call the callback function after unlock
	if(bTerminates){
		m_StateCurr = m_StateTerm;
		if(m_TermCallbackFunc){
			m_TermCallbackFunc(m_pcTermCallbackContext);
		}
	}
	if(!bFound){
		OVNC_DEBUG_WARN("State machine: No matching state found.");
		return vncError_InvalidState;
	}
	
	return retExec;
}

vncObjectWrapper<vncStateMachine*> vncStateMachine::New(ovnc_fsm_state_id_t currState, ovnc_fsm_state_id_t termState)
{
	return new vncStateMachine(currState, termState);
}