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
#include "openvnc/network/vncNetUtils.h"
#include "openvnc/vncDebug.h"
#include "openvnc/vncUtils.h"

bool g_bIsBigEngianMachine = false;
static bool g_bInitialized = false;

vncError_t vncNetUtils::Initialize()
{
	if(!g_bInitialized){
		int err = 0;
		static short word = 0x4321;


		// rand()
		srand((unsigned int) vncUtils::GetNow());

		// endianness
		g_bIsBigEngianMachine = ((*(int8_t *)&word) != 0x21);
	#if OVNC_UNDER_WINDOWS
		if(g_bIsBigEngianMachine){
			OVNC_DEBUG_ERROR("Big endian on Windows machine. Is it right?");
		}
	#endif

	#if OVNC_UNDER_WINDOWS
		{
			WORD wVersionRequested;
			WSADATA wsaData;

			wVersionRequested = MAKEWORD(2, 2);

			err = WSAStartup(wVersionRequested, &wsaData);
			if (err != 0) {
				OVNC_DEBUG_FATAL("WSAStartup failed with error: %d\n", err);
				return vncError_SystemError;
			}

			if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2){
				OVNC_DEBUG_FATAL("Could not find a usable version of Winsock.dll\n");
				return vncError_SystemError;
			}
			else{
				g_bInitialized = true;
				OVNC_DEBUG_INFO("The Winsock 2.2 dll was found okay\n");
			}
		}
	#else
		g_bInitialized = tsk_true;
	#endif /* OVNC__UNDER_WINDOWS */
		
		return err ? vncError_SystemError : vncError_Ok;
	}
	return vncError_Ok;
}

vncError_t vncNetUtils::SetBlockingMode(vncNetFd_t nFd, bool bBlocking)
{
	if(nFd != OVNC_NET_INVALID_FD){
#if OVNC_UNDER_WINDOWS
		ULONG mode = bBlocking ? 0 : 1;
		if(ioctlsocket(nFd, FIONBIO, &mode)){
			vncUtils_SystemPrintLastError("ioctlsocket(FIONBIO) have failed");
			return vncError_SystemError;
		}
#else
		int flags;
		if((flags = fcntl(nFd, F_GETFL, 0)) < 0) { 
			vncUtils_SystemPrintLastError("fcntl(F_GETFL) have failed.");
			return vncError_SystemError;
		} 
		if(fcntl(nFd, F_SETFL, flags | (!bBlocking ? O_NONBLOCK : ~O_NONBLOCK)) < 0){ 
			vncUtils_SystemPrintLastError("fcntl(O_NONBLOCK/O_NONBLOCK) have failed.");
			return vncError_SystemError;
		} 
#endif
		return vncError_Ok;
	}
	OVNC_DEBUG_ERROR("Invalid parameter");
	return vncError_InvalidParameter;
}

vncError_t vncNetUtils::GetSockAddr(vncNetFd_t nFd, struct sockaddr_storage *pResult)
{
	if(nFd != OVNC_NET_INVALID_FD){
		socklen_t namelen = sizeof(*pResult);
		return getsockname(nFd, (struct sockaddr*)pResult, &namelen) ? vncError_SystemError : vncError_Ok;
	}
	OVNC_DEBUG_ERROR("Invalid parameter");
	return vncError_InvalidParameter;
}

vncError_t vncNetUtils::SockAddrInit(const char *pHost, vncNetPort_t nPort, vncNetworkType_t eType, struct sockaddr_storage *addr)
{
	vncError_t ret;
	struct sockaddr_storage ai_addr;

	if((ret = vncNetUtils::SockAddrInfoInit(pHost, nPort, eType, &ai_addr, 0, 0, 0)) != vncError_Ok){
		return ret;
	}
	
	memcpy(addr, &ai_addr, sizeof(ai_addr));

	return ret;
}

vncError_t vncNetUtils::GetIpAndPort(struct sockaddr *pAddr, vncNetIP_t *pIP, vncNetPort_t *pPort)
{
	int status = -1;

	if(pAddr->sa_family == AF_INET){
		struct sockaddr_in *sin = (struct sockaddr_in *)pAddr;
		if(pPort){
			*pPort = vncNetUtils::NetworkToHostShort(sin->sin_port);
			status = 0;
		}
		if(pIP){
			if((status = ovnc_net_getnameinfo((struct sockaddr*)sin, sizeof(*sin), *pIP, sizeof(*pIP), 0, 0, NI_NUMERICHOST))){
				return vncError_SystemError;
			}
		}
	}
	else if(pAddr->sa_family == AF_INET6)
	{
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)pAddr;
#if OVNC_UNDER_WINDOWS
		int index;
#endif
		if(pPort){
			*pPort = vncNetUtils::NetworkToHostShort(sin6->sin6_port);
			status = 0;
		}
		if(pIP){
			if((status = ovnc_net_getnameinfo((struct sockaddr*)sin6, sizeof(*sin6), *pIP, sizeof(*pIP), 0, 0, NI_NUMERICHOST))){
				return vncError_SystemError;
			}

#if OVNC_UNDER_WINDOWS
			if((index = vncUtils::IndexOf(*pIP, vncUtils::StringLength(*pIP), "%")) > 0){
				*(*pIP + index) = '\0';
			}
#endif
		}
	}
	else
	{
		OVNC_DEBUG_ERROR("Unsupported address family");
		return vncError_SystemError;
	}

	return status ? vncError_SystemError : vncError_Ok;
}

vncError_t vncNetUtils::GetIpAndPort(vncNetFd_t nFd, vncNetIP_t *pIP, vncNetPort_t *pPort)
{
	if(pPort){
		*pPort = 0;
	}

	if(nFd != OVNC_NET_INVALID_FD){
		int status;
		struct sockaddr_storage ss;
		if((status = vncNetUtils::GetSockAddr(nFd, &ss)) != vncError_Ok){
			OVNC_DEBUG_ERROR("OVNC_GET_SOCKADDR has failed with status code: %d", status);
			return vncError_SystemError;
		}

		return vncNetUtils::GetIpAndPort(((struct sockaddr *)&ss), pIP, pPort);
	}

	OVNC_DEBUG_ERROR("Invalid parameter");
	return vncError_InvalidParameter;
}

vncError_t vncNetUtils::SockFdInit(const char *pHost, vncNetPort_t nPort, vncNetworkType_t eType, vncNetFd_t *pFd)
{
	int status = -1;
	struct sockaddr_storage ai_addr;
	int ai_family, ai_socktype, ai_protocol;
	*pFd = OVNC_NET_INVALID_FD;
	vncError_t ret = vncError_SystemError;
	
	if((ret = vncNetUtils::SockAddrInfoInit(pHost, nPort, eType, &ai_addr, &ai_family, &ai_socktype, &ai_protocol)) != vncError_Ok){
		vncUtils_SystemPrintLastError("SockAddrInfoInit failed");
		return ret;
	}
	
	if((*pFd = socket(ai_family, ai_socktype, ai_protocol)) == OVNC_NET_INVALID_FD){
		vncUtils_SystemPrintLastError("Failed to create new socket.");
		goto bail;
	}
#if OVNC_USE_POLL // For win32 WSA* function the socket is auto. set to nonblocking mode
	if((ret = vncUtils::SetBlockingMode(*pFd, false)) != vncError_Ok){
		return ret;
	}
#endif
	
#if OVNC_HAVE_SS_LEN
	if((status = bind(*pFd, (const struct sockaddr*)&ai_addr, ai_addr.ss_len)))
#else
	if((status = bind(*pFd, (const struct sockaddr*)&ai_addr, sizeof(ai_addr))))
#endif
	{
		vncUtils_SystemPrintLastError("Bind have failed");
		vncNetUtils::CloseSocketFd(pFd);
		
		goto bail;
	}
	
bail:
	return (*pFd == OVNC_NET_INVALID_FD) ? vncError_SystemError : vncError_Ok;
}

vncError_t vncNetUtils::SockAddrInfoInit(const char *pHost, vncNetPort_t nPort, vncNetworkType_t eType, struct sockaddr_storage *ai_addr, int *ai_family, int *ai_socktype, int *ai_protocol)
{
	int status = 0;
	struct addrinfo *result = 0;
	struct addrinfo *ptr = 0;
	struct addrinfo hints;
	vncNumberString_t p;
	
	vncUtils::IntegerToString(nPort, &p);

	// hints address info structure
	memset(&hints, 0, sizeof(hints));
	//hints.ai_family =  OVNC_SOCKET_TYPE_IS_IPV46(type) ? AF_UNSPEC : (OVNC_SOCKET_TYPE_IS_IPV6(type) ? AF_INET6 : AF_INET);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = vncNetUtils::IsStream(eType) ? SOCK_STREAM : SOCK_DGRAM;
	hints.ai_protocol = vncNetUtils::IsStream(eType) ? IPPROTO_TCP : IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	// Performs getaddrinfo
	if((status = getaddrinfo(pHost, p, &hints, &result))){
		vncUtils_SystemPrintLastError("getaddrinfo have failed");
		goto bail;
	}
	
	// Find our address
	for(ptr = result; ptr; ptr = ptr->ai_next){
		/* Only IPv4 and IPv6 are supported */
		if(ptr->ai_family != AF_INET6 && ptr->ai_family != AF_INET){
			continue;
		}
		/* duplicate addrinfo ==> Bad idea
		*ai = tsk_calloc(1, sizeof (struct addrinfo));
		memcpy (*ai, ptr, sizeof (struct addrinfo));
		(*ai)->ai_addr = tsk_calloc(1, ptr->ai_addrlen);
		memcpy((*ai)->ai_addr, ptr->ai_addr, ptr->ai_addrlen);
		(*ai)->ai_addrlen = ptr->ai_addrlen;
		(*ai)->ai_next = 0;
		(*ai)->ai_canonname = 0;*/

		if(ai_addr)memcpy(ai_addr, ptr->ai_addr, ptr->ai_addrlen);
		if(ai_family) *ai_family = ptr->ai_family;
		if(ai_socktype) *ai_socktype = ptr->ai_socktype;
		if(ai_protocol) *ai_protocol = ptr->ai_protocol;
		
		/* We prefer IPv4 but IPv6 can also work */
		if(ptr->ai_family == AF_INET){
			break;
		}
	}

bail:
	ovnc_freeaddrinfo(result);

	return status ? vncError_SystemError : vncError_Ok;
}

vncError_t vncNetUtils::SockFdConnectTo(vncNetFd_t nFd, const struct sockaddr_storage *to)
{
	int status = -1;

#if OVNC_UNDER_WINDOWS

	if((status = WSAConnect(nFd, (LPSOCKADDR)to, sizeof(*to), NULL, NULL, NULL, NULL)) == SOCKET_ERROR){
		status = WSAGetLastError();
		if(status == OVNC_NET_ERROR_WOULDBLOCK || status == OVNC_NET_ERROR_ISCONN || status == OVNC_NET_ERROR_INTR || status == OVNC_NET_ERROR_INPROGRESS){
			OVNC_DEBUG_WARN("OVNC_NET_ERROR_WOULDBLOCK/OVNC_NET_ERROR_ISCONN/OVNC_NET_ERROR_INTR/OVNC_NET_ERROR_INPROGRESS  ==> use tnet_sockfd_waitUntilWritable.");
			status = 0;
		}
		else{
			vncUtils_SystemPrintLastError("WSAConnect have failed.");
		}
	}

#else /* !OVNC_UNDER_WINDOWS */

#if OVNC_HAVE_SS_LEN
		if((status = connect(nFd, (struct sockaddr*)to, to->ss_len)))
#	else
		if((status = connect(nFd, (struct sockaddr*)to, sizeof(*to))))
#	endif
		{
			status = vncUtils::SystemGetLastError();
			if(status == OVNC_NET_ERROR_WOULDBLOCK || status == OVNC_NET_ERROR_ISCONN || status == OVNC_NET_ERROR_INPROGRESS || status == OVNC_NET_ERROR_EAGAIN){
				OVNC_DEBUG_WARN("OVNC_NET_ERROR_WOULDBLOCK/OVNC_NET_ERROR_ISCONN/OVNC_NET_ERROR_INPROGRESS/OVNC_NET_ERROR_EAGAIN  ==> use tnet_sockfd_waitUntilWritable.");
				status = 0;
			}
			else{
				vncUtils_SystemPrintLastError("connect have failed.");
			}
		}

#endif /* OVNC_UNDER_WINDOWS */

	return status ? vncError_SystemError : vncError_Ok;
}

vncError_t vncNetUtils::SockfdWaitUntil(vncNetFd_t nFd, long timeout, bool bWritable)
{
	int ret = -1;
	fd_set fds;
	struct timeval timetowait;

	if(nFd == OVNC_NET_INVALID_FD){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}

	if(timeout >=0){
		timetowait.tv_sec = (timeout/1000);
		timetowait.tv_usec = (timeout%1000) * 1000;
	}
	
	FD_ZERO(&fds);
	FD_SET(nFd, &fds);

	ret = select(nFd + 1, bWritable?0:&fds, bWritable?&fds:0, 0, (timeout >=0) ? &timetowait : 0);

	if(ret == 0){ /* timedout */
		return vncError_NetworkError;
	}
	else if(ret == 1/* the total number of socket handles that are ready */){
		return vncError_Ok;
	}
	return vncError_SystemError;
}

vncError_t vncNetUtils::SockfdWaitUntilWritable(vncNetFd_t nFd, long timeout)
{
	return vncNetUtils::SockfdWaitUntil(nFd, timeout, true);
}

vncError_t vncNetUtils::SockfdWaitUntilReadable(vncNetFd_t nFd, long timeout)
{
	return vncNetUtils::SockfdWaitUntil(nFd, timeout, false);
}

vncError_t vncNetUtils::SockFdListen(vncNetFd_t nFd, int nBacklog)
{
	if(nFd != OVNC_NET_INVALID_FD){
		int status = listen(nFd, nBacklog);
		return status ? vncError_SystemError : vncError_Ok;
	}
	OVNC_DEBUG_ERROR("Invalid parameter");
	return vncError_InvalidParameter;
}

unsigned vncNetUtils::SockfdSend(vncNetFd_t nFd, const void* pData, unsigned nSize, int nFlags)
{
	int nRet = -1;
	unsigned nSent = 0;

	if(nFd == OVNC_NET_INVALID_FD){
		OVNC_DEBUG_ERROR("Using invalid FD to send data");
		goto bail;
	}

	while(nSent < nSize){
		if((nRet = send(nFd, (((const char*)pData) + nSent), (nSize - nSent), nFlags)) <= 0){
			if(vncUtils::SystemGetErrno() == OVNC_NET_ERROR_WOULDBLOCK){
				if((vncNetUtils::SockfdWaitUntilWritable(nFd, OVNC_NET_CONNECT_TIMEOUT)) != vncError_Ok){
					break;
				}
				else continue;
			}
			else{
				vncUtils_SystemPrintLastError("Send failed");
				// Under Windows XP if WSAGetLastError()==WSAEINTR then try to disable both the ICS and the Firewall
				// More info about How to disable the ISC: http://support.microsoft.com/?scid=kb%3Ben-us%3B230112&x=6&y=11
				goto bail;
			}
		}
		else{
			nSent += nRet;
		}
	}

bail:
	return nSent;
}