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
#include "openvnc/rfb/vncRfbRectangleData.h"

vncRfbRectangleData::vncRfbRectangleData(vncRfbRectangle_t* pRectangle, const void* pData, unsigned nDataSize)
:vncObject()
{
	m_pData = vncBuffer::New(pData, nDataSize);
	memcpy(&m_Rectangle, pRectangle, sizeof(vncRfbRectangle_t));
}

vncRfbRectangleData::~vncRfbRectangleData()
{
	vncObjectSafeRelease(m_pData);
}

vncObjectWrapper<vncRfbRectangleData*> vncRfbRectangleData::New(vncRfbRectangle_t* pRectangle, const void* pData, unsigned nDataSize)
{
	if(!pRectangle || !pData || !nDataSize){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return NULL;
	}
	return new vncRfbRectangleData(pRectangle, pData, nDataSize);
}