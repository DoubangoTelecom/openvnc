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
#ifndef OVNC_VIDEO_DISPLAY_H
#define OVNC_VIDEO_DISPLAY_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncMutex.h"
#include "openvnc/video/vncVideoFrame.h"
#include "openvnc/rfb/vncRfbEncoding.h"

#include <map>

class OVNC_API vncDisplay : public vncObject
{
protected:
	vncDisplay(unsigned nWidth, unsigned nHeight, void* pParent=NULL);
	
public:
	virtual ~vncDisplay();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncDisplay"; }
	virtual vncObjectWrapper<vncRfbEncoding*> RequestEncoding(vncRfbEncoding_t eEncoding);
	virtual vncError_t AdjustMousePointerToFit(uint16_t &nXPosition, uint16_t &nYPosition);
	virtual bool IsValid();
	virtual vncError_t Display(vncObjectWrapper<vncVideoFrame*>pVNCVideoFrame)=0;

	static vncError_t Initialize();

	static vncObjectWrapper<vncDisplay*> New(unsigned nWidth, unsigned nHeight, void* pParent=NULL);

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable : 4251 )
#endif

protected:
	static bool g_bInitialized;
	unsigned m_nWidth;
	unsigned m_nHeight;
	vncObjectWrapper<vncMutex*>m_pEncodingsMutex;
	std::map<vncRfbEncoding_t, vncObjectWrapper<vncRfbEncoding*> >m_aEncodings;

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
};

#endif /* OVNC_VIDEO_DISPLAY_H */
