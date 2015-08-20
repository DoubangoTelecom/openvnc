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
#ifndef OVNC_RFB_UTILS_H
#define OVNC_RFB_UTILS_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/rfb/vncRfbTypes.h"

class vncRfbUtils
{
public:
	static vncError_t ParseRectangle(const void* pData, unsigned nDataSize, vncRfbRectangle_t &rectangle);

	static OVNC_INLINE bool IsSet(vncRfbPixelFormat_t* pFormat){
		return (pFormat && pFormat->bits_per_pixel && pFormat->depth);
	}
	static OVNC_INLINE vncError_t UnSet(vncRfbPixelFormat_t* pFormat){
		if(!pFormat){
			OVNC_DEBUG_ERROR("Invalid parameter");
			return vncError_InvalidParameter;
		}
		memset(pFormat, 0, sizeof(*pFormat));
		return vncError_Ok;
	}
	static OVNC_INLINE bool IsEqual(const vncRfbPixelFormat_t* pFormat1, const vncRfbPixelFormat_t* pFormat2){
		return (pFormat1 && pFormat2) &&
			(pFormat1->bits_per_pixel == pFormat2->bits_per_pixel) &&
			(pFormat1->depth == pFormat2->depth) &&
			(pFormat1->big_endian_flag == pFormat2->big_endian_flag) &&
			(pFormat1->true_colour_flag == pFormat2->true_colour_flag) &&
			(pFormat1->red_max == pFormat2->red_max) &&
			(pFormat1->green_max == pFormat2->green_max) &&
			(pFormat1->blue_max == pFormat2->blue_max) &&
			(pFormat1->red_shift == pFormat2->red_shift) &&
			(pFormat1->green_shift == pFormat2->green_shift) &&
			(pFormat1->blue_shift == pFormat2->blue_shift);
	}
};

#endif /* OVNC_RFB_UTILS_H */
