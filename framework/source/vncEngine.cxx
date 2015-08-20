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
#include "openvnc/vncEngine.h"
#include "openvnc/network/vncNetUtils.h"
#include "openvnc/video/vncDisplay.h"

bool vncEngine::g_bInitialized = false;

vncEngine::vncEngine()
:vncObject()
{	
	m_bValid = (vncEngine::Initialize() == vncError_Ok);


	
}

vncEngine::~vncEngine()
{
	
}

vncError_t vncEngine::Initialize()
{
	vncError_t ret = vncError_Ok;
	if(!g_bInitialized){
		if((ret = vncNetUtils::Initialize()) != vncError_Ok){
			OVNC_DEBUG_ERROR("vncNetUtils::Initialize failed");
			return ret;
		}
		if((ret = vncDisplay::Initialize()) != vncError_Ok){
			OVNC_DEBUG_ERROR("vncDisplay::Initialize failed");
			return ret;
		}
		g_bInitialized = true;
	}
	return ret;
}

vncObjectWrapper<vncClient*> vncEngine::NewClient(const char* pLocalHost, vncNetPort_t nLocalPort, vncNetworkType_t eType)
{
	vncObjectWrapper<vncClient*> pVNCClient = vncClient::New(pLocalHost, nLocalPort, eType);
	if(pVNCClient && !pVNCClient->IsValid()){
		vncObjectSafeFree(pVNCClient);
	}
	return pVNCClient;
}

vncObjectWrapper<vncServer*> vncEngine::NewServer(const char* pLocalHost, vncNetPort_t nLocalPort, vncNetworkType_t eType)
{
	vncObjectWrapper<vncServer*> pVNCServer = vncServer::New(pLocalHost, nLocalPort, eType);
	if(pVNCServer && !pVNCServer->IsValid()){
		vncObjectSafeFree(pVNCServer);
	}
	return pVNCServer;
}

vncObjectWrapper<vncEngine*> vncEngine::New()
{
	vncObjectWrapper<vncEngine*> pVNCEngine = new vncEngine();
	if(pVNCEngine && !pVNCEngine->IsValid()){
		vncObjectSafeFree(pVNCEngine);
	}
	return pVNCEngine;
}