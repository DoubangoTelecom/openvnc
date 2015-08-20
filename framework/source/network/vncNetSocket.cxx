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
#include "openvnc/network/vncNetSocket.h"
#include "openvnc/network/vncNetUtils.h"
#include "openvnc/vncUtils.h"
#include "openvnc/vncDebug.h"

vncNetSocket::vncNetSocket(const char * pHost, vncNetPort_t nPort, vncNetworkType_t eType, vncNetIPVersion_t eIPversion /*= vncNetIPVersion_IPv4*/, bool bNonBlocking/*=true*/, bool bBindSocket/*=true*/)
:vncObject()
{
	m_eType = eType;
	m_nFD = OVNC_NET_INVALID_FD;
	m_eIPversion = eIPversion;
	m_nPort = 0;

	
	int status;
	vncNumberString_t aPort;
	struct addrinfo *result = 0;
	struct addrinfo *ptr = 0;
	struct addrinfo hints;
	vncNetHost_t aLocalHostname;
	
	if(vncUtils::IntegerToString(nPort, &aPort)){
		OVNC_DEBUG_ERROR("%d not a valid network port", nPort);
		return;
	}

	memset(aLocalHostname, 0, sizeof(aLocalHostname));

	// Get the local pHost name
	if(pHost != OVNC_NET_SOCKET_HOST_ANY && !vncUtils::StringIsNullEmpty(pHost)){
		memcpy(aLocalHostname, pHost, vncUtils::StringLength(pHost) > sizeof(aLocalHostname) - 1 ? sizeof(aLocalHostname) - 1 : vncUtils::StringLength(pHost));
	}
	else{
		if(m_eIPversion == vncNetIPVersion_IPv6){
			memcpy(aLocalHostname, "::", 2);
		}
		else{
			memcpy(aLocalHostname, "0.0.0.0", 7);
		}
	}

	// hints address info structure 
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = eIPversion == vncNetIPVersion_IPv4and6 ? AF_UNSPEC : (eIPversion == vncNetIPVersion_IPv6 ? AF_INET6 : AF_INET);
	hints.ai_socktype = vncNetUtils::IsStream(eType) ? SOCK_STREAM : SOCK_DGRAM;
	hints.ai_protocol = vncNetUtils::IsStream(eType) ? IPPROTO_TCP : IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE
#if !TNET_UNDER_WINDOWS || _WIN32_WINNT>=0x600
		| AI_ADDRCONFIG
#endif
		;

	// Performs getaddrinfo
	if((status = ovnc_net_getaddrinfo(aLocalHostname, aPort, &hints, &result))){
		vncUtils_SystemPrintLastError("tnet_getaddrinfo(family=%d, pHostname=%s and port=%s) failed: [%s]", 
			hints.ai_family, aLocalHostname, aPort, ovnc_net_gai_strerror(status));
		goto bail;
	}

	/* Find our address. */
	for(ptr = result; ptr; ptr = ptr->ai_next){
		m_nFD = ovnc_net_socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if(ptr->ai_family != AF_INET6 && ptr->ai_family != AF_INET){
			continue;
		}

		if(bBindSocket){
			// Bind the socket
			if((status = bind(m_nFD, ptr->ai_addr, ptr->ai_addrlen))){
				vncUtils_SystemPrintLastError("bind have failed.");
				Close();
				continue;
			}

			// Get local IP string
			if((status = vncNetUtils::GetIpAndPort(m_nFD , &m_aIP, &m_nPort)) != vncError_Ok){
				vncUtils_SystemPrintLastError("Failed to get local IP and port.");
				Close();
				continue;
			}
		}

		// Sets the real socket type (if ipv46)
		if(ptr->ai_family == AF_INET6) {
			m_eIPversion = vncNetIPVersion_IPv6;
		}
		else{
			m_eIPversion = vncNetIPVersion_IPv4;
		}
		break;
	}

	// Check socket validity
	if(!IsValid()) {
		vncUtils_SystemPrintLastError("Invalid socket");
		goto bail;
	}		

	/* To avoid "Address already in use" error */
	{
#if defined(SOLARIS)
		char yes = '1';
#else
		int yes = 1;
#endif
		if(setsockopt(m_nFD, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int))){
			vncUtils_SystemPrintLastError("setsockopt(SO_REUSEADDR) have failed.");
		}
	}

	/* Set SND and RCV buffers size */
#if 0 /* WSAENOBUFS */
	{
		static int rcvBuffSize = OVNC_NET_RCVBUF_SIZE;
		static int sndBuffSize = OVNC_NET_SNDBUF_SIZE;
		int ret;
		if((ret = setsockopt(m_nFD, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuffSize, sizeof(rcvBuffSize)))){
			vncUtils_SystemPrintLastError("setsockopt(SOL_SOCKET, SO_RCVBUF) has failed with error code %d", ret);
		}
		if((ret = setsockopt(m_nFD, SOL_SOCKET, SO_SNDBUF, (char*)&sndBuffSize, sizeof(sndBuffSize)))){
			vncUtils_SystemPrintLastError("setsockopt(SOL_SOCKET, SO_RCVBUF) has failed with error code %d", ret);
		}
	}
#endif

	/* Sets the socket to nonblocking mode */
	if(bNonBlocking){
		if((status = vncNetUtils::SetBlockingMode(m_nFD, !bNonBlocking)) != vncError_Ok){
			goto bail;
		}
	}

bail:
	// Free addrinfo
	ovnc_freeaddrinfo(result);

	// Close socket if failed. */
	if(status && IsValid()){
		Close();
	}
}

vncNetSocket::~vncNetSocket()
{
	Close();
}

vncError_t vncNetSocket::Close()
{
	int ret = 0;
	if(m_nFD != OVNC_NET_INVALID_FD){
		ret = vncNetUtils::CloseSocketFd(&m_nFD);
	}
	return ret ? vncError_SystemError : vncError_Success;
}

bool vncNetSocket::IsValid()
{
	return (m_eType != vncNetworkType_None && m_nFD != OVNC_NET_INVALID_FD);
}

vncObjectWrapper<vncNetSocket*> vncNetSocket::New(const char * pHost, vncNetPort_t nPort, vncNetworkType_t eType, vncNetIPVersion_t eIPversion /*= vncNetIPVersion_IPv4*/, bool bNonBlocking/*=true*/, bool bBindSocket/*=true*/)
{
	vncObjectWrapper<vncNetSocket*> pVNCSocket = new vncNetSocket(pHost, nPort, eType, eIPversion, bNonBlocking, bBindSocket);
	if(pVNCSocket && !pVNCSocket->IsValid()){
		vncObjectSafeFree(pVNCSocket);
	}
	return pVNCSocket;
}
