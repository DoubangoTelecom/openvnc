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
#ifndef OVNC_ENGINE_H
#define OVNC_ENGINE_H

#include "vncConfig.h"
#include "openvnc/vncCommon.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncClient.h"
#include "openvnc/vncServer.h"

class OVNC_API vncEngine : public vncObject
{
protected:
	vncEngine();
public:
	virtual ~vncEngine();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncEngine"; }
	virtual OVNC_INLINE bool IsValid(){ return m_bValid; }

	static vncError_t Initialize();

	virtual vncObjectWrapper<vncClient*> NewClient(const char* pLocalHost = OVNC_NET_SOCKET_HOST_ANY, vncNetPort_t nLocalPort = OVNC_NET_SOCKET_PORT_ANY, vncNetworkType_t eType = vncNetworkType_TCP);
	virtual vncObjectWrapper<vncServer*> NewServer(const char* pLocalHost = OVNC_NET_SOCKET_HOST_ANY, vncNetPort_t nLocalPort = OVNC_CONFIG_SERVER_PORT, vncNetworkType_t eType = vncNetworkType_TCP);

	static vncObjectWrapper<vncEngine*> New();

private:
	bool m_bValid;
	static bool g_bInitialized;
};

#endif /* OVNC_ENGINE_H */
