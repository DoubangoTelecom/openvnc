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
#ifndef OVNC_BYTE_WRITER_H
#define OVNC_BYTE_WRITER_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"

class vncByteWriter : public vncObject
{
public:
	vncByteWriter(const void* pData=NULL, unsigned nDataSize=0);
	virtual ~vncByteWriter();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncByteWriter"; }
	virtual vncError_t SetData(const void* pcData, unsigned nDataSize);
	virtual vncError_t Write(uint32_t nVal, unsigned nBytesCount, int32_t nIndex = -1, bool bMoveIndex = true);
	virtual OVNC_INLINE const void* GetData(){ return m_pcData; }
	virtual OVNC_INLINE unsigned GetDataSize(){ return m_nDataSize; }
	virtual OVNC_INLINE unsigned GetDataIndex(){ return m_nDataIndex; }

private:
	const void* m_pcData;
	unsigned m_nDataSize;
	unsigned m_nDataIndex;
};

#endif /* OVNC_BYTE_WRITER_H */

