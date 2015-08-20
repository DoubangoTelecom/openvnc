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
#ifndef OVNC_RFB_RECTANGLE_DATA_H
#define OVNC_RFB_RECTANGLE_DATA_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncBuffer.h"
#include "openvnc/rfb/vncRfbTypes.h"

class vncRfbRectangleData : public vncObject
{
protected:
	vncRfbRectangleData(vncRfbRectangle_t* pRectangle, const void* pData, unsigned nDataSize);
public:
	virtual ~vncRfbRectangleData();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncRfbRectangleData"; }
	virtual OVNC_INLINE vncObjectWrapper<vncBuffer*> GetData(){ return m_pData; }
	virtual OVNC_INLINE vncRfbRectangle_t* GetRectangle(){ return &m_Rectangle; }
	
	static vncObjectWrapper<vncRfbRectangleData*> New(vncRfbRectangle_t* pRectangle, const void* pData, unsigned nDataSize);

private:
	vncRfbRectangle_t m_Rectangle;
	vncObjectWrapper<vncBuffer*>m_pData;
};

#endif /* OVNC_RFB_RECTANGLE_DATA_H */
