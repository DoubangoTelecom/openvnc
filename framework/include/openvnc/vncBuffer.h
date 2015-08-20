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
#ifndef OVNC_BUFFER_H
#define OVNC_BUFFER_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"

class vncBuffer : public vncObject
{
protected:
	vncBuffer(const void* pData=NULL, unsigned nDataSize=0);
public:
	virtual ~vncBuffer();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncBuffer"; }
	virtual OVNC_INLINE const void* GetData(){ return m_pData; }
	virtual OVNC_INLINE unsigned GetDataSize(){ return m_nDataSize; }

	vncError_t CleanUp();
	vncError_t Append(const void* pData, unsigned nSize);
	vncError_t Realloc(unsigned nSize);
	vncError_t Remove(unsigned nPosition, unsigned nSize);
	vncError_t Copy(unsigned nStart, const void* pData, unsigned nSize);
	vncError_t TakeOwnership(void** ppData, unsigned nSize);
	
	static vncObjectWrapper<vncBuffer*> New(const void* pData=NULL, unsigned nDataSize=0);
	
private:
	void *m_pData;
	unsigned m_nDataSize;
};

#endif /* OVNC_BUFFER_H */
