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
#include "openvnc/rfb/vncRfbUtils.h"
#include "openvnc/vncUtils.h"
#include "openvnc/network/vncNetUtils.h"


vncError_t vncRfbUtils::ParseRectangle(const void* pData, unsigned nDataSize, vncRfbRectangle_t &rectangle)
{
	// nDataSize == 0 Must not return Invalid Parameter
	if(!pData){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	if(nDataSize < OVNC_RFB_RECTANGLE_HEADER_SIZE){
		return vncError_TooShort;
	}

	const uint8_t* _pData = reinterpret_cast<const uint8_t*>(pData);
	rectangle.x_position = vncNetUtils::NetworkToHostPShort(&_pData[0]);
	rectangle.y_position = vncNetUtils::NetworkToHostPShort(&_pData[2]);
	rectangle.width = vncNetUtils::NetworkToHostPShort(&_pData[4]);
	rectangle.height = vncNetUtils::NetworkToHostPShort(&_pData[6]);
	rectangle.encoding_type = (vncRfbEncoding_t)vncNetUtils::NetworkToHostPLong(&_pData[8]);

	return vncError_Ok;
}
