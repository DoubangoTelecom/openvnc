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
#ifndef OVNC_NET_UTILS_H
#define OVNC_NET_UTILS_H

#include "vncConfig.h"
#include "openvnc/vncCommon.h"


#if OVNC_UNDER_WINDOWS
#	include	<winsock2.h>
#	include	<ws2tcpip.h>
#	include <iphlpapi.h>
#else
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <sys/select.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <fcntl.h>
#	include <sys/ioctl.h>
#	include <unistd.h>
#	include <net/if.h>
#	if OVNC_HAVE_IFADDRS
#		include <ifaddrs.h>
#	endif
#	if OVNC_HAVE_POLL
#		include <poll.h>
#	endif /* OVNC_HAVE_POLL */
#endif

class vncNetUtils
{
public:
	static vncError_t Initialize();
	static OVNC_INLINE bool IsStream(vncNetworkType_t eType) { return eType == vncNetworkType_TCP || eType == vncNetworkType_TLS; }
	static OVNC_INLINE bool IsDGram(vncNetworkType_t eType){ return eType == vncNetworkType_UDP; }
	static OVNC_INLINE int CloseSocketFd(vncNetFd_t *pFd)
	{
		if(*pFd != OVNC_NET_INVALID_FD){
			int ret;
	#if OVNC_UNDER_WINDOWS
			ret = closesocket(*pFd);
	#else
			ret = close(*pFd);
	#endif

			*pFd = OVNC_NET_INVALID_FD;
			return ret;
		}
		return 0;
	}
	static OVNC_INLINE int CloseSocketFd(vncNetFd_t pFd)
	{
		return vncNetUtils::CloseSocketFd(&pFd);
	}

	static OVNC_INLINE unsigned short HostToNetworkShort(unsigned short x)
	{
		if(vncNetUtils::IsBigEndian()){
			return x;
		}
		else{
			return ((((uint16_t)(x) & 0xff00) >> 8)		|
							(((uint16_t)(x) & 0x00ff) << 8));
		}
	}
	static OVNC_INLINE unsigned short HostToNetworkPShort(const void* px)
	{
		unsigned short y = OVNC_TO_UINT16((const uint8_t*)px);
		return HostToNetworkShort(y);
	}
	static OVNC_INLINE unsigned long HostToNetworkLong(unsigned long x)
	{
		if(vncNetUtils::IsBigEndian()){
			return x;
		}
		else{
			return ((((uint32_t)(x) & 0xff000000) >> 24)	| \
							(((uint32_t)(x) & 0x00ff0000) >> 8)		| \
							(((uint32_t)(x) & 0x0000ff00) << 8)		| \
							(((uint32_t)(x) & 0x000000ff) << 24));
		}
	}
	static OVNC_INLINE unsigned long HostToNetworkPLong(const void* px)
	{
		unsigned long y = OVNC_TO_UINT32((const uint8_t*)px);
		return HostToNetworkLong(y);
	}
	static OVNC_INLINE bool IsBigEndian()
	{
		extern bool g_bIsBigEngianMachine;
#if OVNC_LITTLE_ENDIAN
	return false;
#elif OVNC_BIG_ENDIAN
	return true;
#else
	return g_bIsBigEngianMachine;
#endif 
	}
	static OVNC_INLINE unsigned short NetworkToHostShort(unsigned short x){ return vncNetUtils::HostToNetworkShort(x); }
	static OVNC_INLINE unsigned short NetworkToHostPShort(const void* px){ return vncNetUtils::HostToNetworkPShort(px); }
	static OVNC_INLINE unsigned long NetworkToHostLong(unsigned long x){ return vncNetUtils::HostToNetworkLong(x); }
	static OVNC_INLINE unsigned long NetworkToHostPLong(const void* px){ return vncNetUtils::HostToNetworkPLong(px); }
	
	static vncError_t SetBlockingMode(vncNetFd_t nFd, bool bBlocking);
	static vncError_t GetSockAddr(vncNetFd_t nFd, struct sockaddr_storage *pResult);
	static vncError_t SockAddrInit(const char *pHost, vncNetPort_t nPort, vncNetworkType_t eType, struct sockaddr_storage *addr);
	static vncError_t GetIpAndPort(struct sockaddr *pAddr, vncNetIP_t *pIP, vncNetPort_t *pPort);
	static vncError_t GetIpAndPort(vncNetFd_t nFd, vncNetIP_t *pIP, vncNetPort_t *pPort);
	static vncError_t SockFdInit(const char *pHost, vncNetPort_t nPort, vncNetworkType_t eType, vncNetFd_t *pFd);
	static vncError_t SockAddrInfoInit(const char *pHost, vncNetPort_t nPort, vncNetworkType_t eType, struct sockaddr_storage *ai_addr, int *ai_family, int *ai_socktype, int *ai_protocol);
	static vncError_t SockFdConnectTo(vncNetFd_t nFd, const struct sockaddr_storage *to);
	static vncError_t SockFdListen(vncNetFd_t nFd, int nBacklog);
	static vncError_t SockfdWaitUntil(vncNetFd_t nFd, long timeout, bool bWritable=false);
	static vncError_t SockfdWaitUntilWritable(vncNetFd_t nFd, long timeout);
	static vncError_t SockfdWaitUntilReadable(vncNetFd_t nFd, long timeout);
	static unsigned SockfdSend(vncNetFd_t nFd, const void* pData, unsigned nSize, int nFlags);
};


#endif /* OVNC_NET_UTILS_H */
