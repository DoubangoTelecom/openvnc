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
#include "openvnc/video/vncScreenGrabber.h"
#include "openvnc/video/vncScreenGrabberGDI.h"

bool vncScreenGrabber::g_bInitialized = false;

vncScreenGrabber::vncScreenGrabber(const void* pParent)
:vncObject()
{
	m_pEncodingsMutex = vncMutex::New(false);
}

vncScreenGrabber::~vncScreenGrabber()
{
	m_pEncodingsMutex->Lock();
	m_aEncodings.clear();
	m_pEncodingsMutex->UnLock();

	vncObjectSafeFree(m_pEncodingsMutex);
}

vncObjectWrapper<vncRfbEncoding*> vncScreenGrabber::RequestEncoding(vncRfbEncoding_t eEncoding)
{
	vncObjectWrapper<vncRfbEncoding*>pVNCEncoding;

	m_pEncodingsMutex->Lock();
	
	std::map<vncRfbEncoding_t, vncObjectWrapper<vncRfbEncoding*> >::iterator iter 
		= m_aEncodings.find(eEncoding);
	if(iter != m_aEncodings.end()){
		pVNCEncoding = iter->second;
	}
	else{
		if((pVNCEncoding = vncRfbEncoding::New(eEncoding))){
			m_aEncodings.insert(std::pair<vncRfbEncoding_t, vncObjectWrapper<vncRfbEncoding*> >(pVNCEncoding->GetEncoding(), pVNCEncoding));
		}
	}
	
	m_pEncodingsMutex->UnLock();

	return pVNCEncoding;
}

vncObjectWrapper<vncScreenGrabber*> vncScreenGrabber::New(const void* pParent)
{
	vncObjectWrapper<vncScreenGrabber*> pVNCGrabber;

#if OVNC_UNDER_WINDOWS
	{
		vncObjectWrapper<vncScreenGrabber*> pVNCGrabberGDI = new vncScreenGrabberGDI(pParent);
		if(pVNCGrabberGDI && pVNCGrabberGDI->IsValid()){
			pVNCGrabber = dynamic_cast<vncScreenGrabber*>(*pVNCGrabberGDI);
		}
	}
#else
	{
		OVNC_DEBUG_ERROR("Not implemented");
	}
#endif

	return pVNCGrabber;
}

vncError_t vncScreenGrabber::Initialize()
{
	vncError_t ret = vncError_Ok;

	if(!g_bInitialized){
#if OVNC_UNDER_WINDOWS		
		g_bInitialized = (vncScreenGrabberGDI::Initialize() == TRUE);
#else
	g_bInitialized = true;
#endif
	}

	return ret;
}