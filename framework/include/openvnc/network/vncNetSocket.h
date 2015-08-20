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
#ifndef OVNC_NET_SOCKET_H
#define OVNC_NET_SOCKET_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"

class vncNetSocket : public vncObject
{
protected:
	vncNetSocket(const char * pHost, vncNetPort_t port, vncNetworkType_t eType, vncNetIPVersion_t eIPversion = vncNetIPVersion_IPv4, bool bNonBlocking=true, bool bBindSocket=true);
public:
	virtual ~vncNetSocket();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncNetSocket"; }
	virtual OVNC_INLINE bool IsValid();
	virtual OVNC_INLINE vncNetIP_t* GetIP(){ return &m_aIP; }
	virtual OVNC_INLINE vncNetPort_t GetPort(){ return m_nPort; }
	virtual OVNC_INLINE vncNetworkType_t GetType(){ return m_eType; }
	virtual OVNC_INLINE vncNetFd_t GetFd(){ return m_nFD; }
	static vncObjectWrapper<vncNetSocket*> New(const char * pHost, vncNetPort_t port, vncNetworkType_t eType, vncNetIPVersion_t eIPversion = vncNetIPVersion_IPv4, bool bNonBlocking=true, bool bBindSocket=true);

private:
	vncError_t Close();

private:
	vncNetworkType_t m_eType;
	vncNetIPVersion_t m_eIPversion;
	vncNetFd_t m_nFD;
	vncNetIP_t m_aIP;
	vncNetPort_t m_nPort;
};

#endif /* OVNC_NET_SOCKET_H */
