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
#ifndef OVNC_TYPES_H
#define OVNC_TYPES_H

#include "vncConfig.h"

typedef char vncNumberString_t[21];
#ifdef _WIN32_WCE
	typedef TCHAR vncSystemError_t[512];
#else
	typedef char vncSystemError_t[512];
#endif

typedef enum vncError_e
{
	vncError_Success = 0,
	vncError_Ok = vncError_Success,

	vncError_NotImplemented,
	vncError_InvalidParameter,
	vncError_InvalidState,
	vncError_InvalidData,
	vncError_TooShort,
	vncError_SystemError,
	vncError_ApplicationError,
	vncError_3rdApplicationError,
	vncError_MemoryAllocFailed,
	vncError_BufferOverflow,
	vncError_NetworkError
}
vncError_t;


#define OVNC_MIN(a,b)					(((a) < (b)) ? (a) : (b))
#define OVNC_MIN_3(a,b,c)				OVNC_MIN(OVNC_MIN(a,b),c)
#define OVNC_MAX(a,b)					(((a) > (b)) ? (a) : (b))
#define OVNC_ABS(a)						(((a)< 0) ? -(a) : (a))
#define OVNC_CLAMP(nMin, nVal, nMax)		((nVal) > (nMax)) ? (nMax) : (((nVal) < (nMin)) ? (nMin) : (nVal))

// used to avoid doing *((uint32_t*)ptr) which don't respect memory alignment on
// some embedded (ARM,?...) platforms
#define OVNC_TO_UINT32(u8_ptr) (((uint32_t)(u8_ptr)[0]) | ((uint32_t)(u8_ptr)[1])<<8 | ((uint32_t)(u8_ptr)[2])<<16 | ((uint32_t)(u8_ptr)[3])<<24)
#define OVNC_TO_INT32(u8_ptr) (((int32_t)(u8_ptr)[0]) | ((int32_t)(u8_ptr)[1])<<8 | ((int32_t)(u8_ptr)[2])<<16 | ((int32_t)(u8_ptr)[3])<<24)
#define OVNC_TO_UINT16(u8_ptr) (((uint16_t)(u8_ptr)[0]) | ((uint16_t)(u8_ptr)[1])<<8)

#define OVNC_SAFE_FREE(ptr) (void)vncUtils::Free((void**)(&ptr));
#define OVNC_FREE(ptr) OVNC_SAFE_FREE(ptr)



#if defined(_MSC_VER)
#	define snprintf		_snprintf
//#	define vsnprintf	_vsnprintf
#	define strdup		_strdup
#	define stricmp		_stricmp
#	define strnicmp		_strnicmp
#else
#	if !HAVE_STRNICMP && !HAVE_STRICMP
#	define stricmp		strcasecmp
#	define strnicmp		strncasecmp
#	endif
#endif

#if defined(va_copy)
#	define ovnc_va_copy(D, S)       va_copy((D), (S))
#elif defined(__va_copy)
#	define ovnc_va_copy(D, S)       __va_copy((D), (S))
#else
#	define ovnc_va_copy(D, S)       ((D) = (S))
#endif

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

#if defined(OVNC_HAVE_SCTP)
#include <netinet/sctp.h>
#endif

typedef enum vncNetworkType_e
{
	vncNetworkType_None,

	vncNetworkType_UDP,
	vncNetworkType_TCP,
	vncNetworkType_TLS
}
vncNetworkType_t;

typedef enum vncNetIPVersion_e
{
	vncNetIPVersion_None = 0x00,

	vncNetIPVersion_IPv4 = 0x01<<0,
	vncNetIPVersion_IPv6 = 0x01<<1,
	vncNetIPVersion_IPv4and6 = vncNetIPVersion_IPv4 | vncNetIPVersion_IPv6
}
vncNetIPVersion_t;

#if OVNC_UNDER_WINDOWS
#	define OVNC_NET_INVALID_SOCKET				INVALID_SOCKET
#	define OVNC_NET_ERROR_WOULDBLOCK			WSAEWOULDBLOCK
#	define OVNC_NET_ERROR_INPROGRESS			WSAEINPROGRESS
#	define OVNC_NET_ERROR_CONNRESET				WSAECONNRESET
#	define OVNC_NET_ERROR_INTR					WSAEINTR
#	define OVNC_NET_ERROR_ISCONN				WSAEISCONN
#	define OVNC_NET_ERROR_EAGAIN				OVNC_NET_ERROR_WOULDBLOCK /* WinSock FIX */
#	define ovnc_net_ioctlt ioctlsocket /* FIXME: use WSAIoctl */
#	define ovnc_net_socket(family, type, protocol) WSASocket((family), (type), (protocol), NULL, 0, WSA_FLAG_OVERLAPPED)
#	if defined(_WIN32_WCE)
#		define ovnc_net_gai_strerror(...)		"FIXME"
#	else
#		define ovnc_net_gai_strerror			gai_strerrorA
#	endif
#else
#	define OVNC_NET_INVALID_SOCKET				-1
#	define OVNC_NET_ERROR_WOULDBLOCK			EWOULDBLOCK
#	define OVNC_NET_ERROR_INPROGRESS			EINPROGRESS
#	define OVNC_NET_ERROR_CONNRESET				ECONNRESET
#	define OVNC_NET_ERROR_INTR					EINTR
#	define OVNC_NET_ERROR_ISCONN				EISCONN
#	define OVNC_NET_ERROR_EAGAIN				EAGAIN
#	define ovnc_net_gai_strerror				gai_strerror
#	define ovnc_net_ioctlt ioctl
#	define ovnc_net_soccket(family, type, protocol) socket((family), (type), (protocol))
#endif
#define ovnc_net_getnameinfo				getnameinfo
#define ovnc_net_getaddrinfo				getaddrinfo
#define ovnc_freeaddrinfo(ai)				if(ai)freeaddrinfo(ai)
#define OVNC_NET_INVALID_FD					OVNC_NET_INVALID_SOCKET
#define OVNC_NET_SOCKET_HOST_ANY			NULL
#define OVNC_NET_SOCKET_PORT_ANY			0
#define OVNC_NET_CONNECT_TIMEOUT			2000
#define OVNC_NET_MAX_FDS					64
#define OVNC_NET_RCVBUF_SIZE				(64 * 1024)
#define OVNC_NET_SNDBUF_SIZE				(64 * 1024)

#if OVNC_HAVE_SA_LEN
#	define ovnc_get_sockaddr_size(psockaddr)	(psockaddr)->sa_len
#else
#	define ovnc_get_sockaddr_size(psockaddr)	((psockaddr)->sa_family == AF_INET6 ? sizeof(struct sockaddr_in6): ((psockaddr)->sa_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(*(psockaddr))))
#endif

#define vncNetError_t vncSystemError_t

typedef int32_t vncNetFd_t;
typedef uint16_t vncNetPort_t;
typedef int32_t vncNetFamily_t;
typedef char vncNetHost_t[NI_MAXHOST];
typedef char vncNetIP_t[INET6_ADDRSTRLEN];

#endif /* OVNC_TYPES_H */
