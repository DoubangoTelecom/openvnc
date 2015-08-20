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
#ifndef OVNC_THREAD_H
#define OVNC_THREAD_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"

class vncThread : public vncObject
{
protected:
	vncThread(void *(*start) (void *), void *arg);

public:
	virtual ~vncThread();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncThread"; }
	virtual OVNC_INLINE bool IsValid();
	virtual vncError_t SetPriority(int32_t nPriority);
	virtual vncError_t Join();

	static vncError_t SetPriority(void* pTid, int32_t nPriority);
	static vncError_t SetPriority2(int32_t nPriority);
	static vncError_t Sleep(uint64_t nMilliseconds);

	static vncObjectWrapper<vncThread*> New(void *(*start) (void *), void *arg);
private:
	void* m_pTid;
};

#endif /* OVNC_THREAD_H */
