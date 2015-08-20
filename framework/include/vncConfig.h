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
#ifndef OVNC_CONFIG_H
#define OVNC_CONFIG_H

#if defined(WIN32) || defined(_WIN32)
#	define OVNC_UNDER_WINDOWS			1
#elif defined(__APPLE__)
#	if TARGET_OS_IPHONE
#		define OVNC_UNDER_IOS			1
#	elif TARGET_OS_MAC
#		define OVNC_UNDER_OSX			1
#	endif
#	define OVNC_UNDER_APPLE				1
#elif ANDROID
#	define OVNC_UNDER_ANDROID			1
#endif

#if OVNC_UNDER_WINDOWS && defined(OVNC_EXPORTS)
# 	define OVNC_API		__declspec(dllexport)
# 	define OVNC_GEXTERN __declspec(dllexport)
#elif OVNC_UNDER_WINDOWS /*&& defined(OVNC_IMPORTS)*/
# 	define OVNC_API __declspec(dllimport)
# 	define OVNC_GEXTERN __declspec(dllimport)
#else
#	define OVNC_API
#	define OVNC_GEXTERN	extern
#endif

// Disable some well-known warnings
#ifdef _MSC_VER
#	define _CRT_SECURE_NO_WARNINGS
#	define OVNC_INLINE	_inline
#else
#	define OVNC_INLINE	inline
#endif

#ifndef INT64_C
#	define INT64_C(val) val##i64
#endif
#ifndef UINT64_C
#	define UINT64_C(val) val##ui64
#endif

#ifndef NULL
#define NULL 0
#endif

// have poll()?
#if (OVNC_UNDER_WINDOWS && (_WIN32_WINNT>=0x0600)) || (OVNC_UNDER_ANDROID) || OVNC_UNDER_APPLE
#	define OVNC_HAVE_POLL		1
#else
#	define OVNC_HAVE_POLL		0
#endif

// whether to use poll()
#if OVNC_UNDER_WINDOWS
#	define OVNC_USE_POLL		0 // Do not use WSAPoll event if under Vista
#else
#	define OVNC_USE_POLL		1
#endif

#if OVNC_UNDER_APPLE
#	define OVNC_HAVE_IFADDRS	1
#   define OVNC_HAVE_DNS_H		1
#	define OVNC_HAVE_SS_LEN		1
#	define OVNC_HAVE_SA_LEN		0
#else
#	define OVNC_HAVE_IFADDRS	0
#   define OVNC_HAVE_DNS_H		0
#	define OVNC_HAVE_SS_LEN		0
#	define OVNC_HAVE_SA_LEN		0
#endif

#if OVNC_UNDER_WINDOWS
#	define OVNC_HAVE_GETTIMEOFDAY				0
#else
#	define OVNC_HAVE_GETTIMEOFDAY				1
#endif

#if OVNC_UNDER_ANDROID
#	define OVNC_HAVE_CLOCK_GETTIME				1
#endif

#define OVNC_CONFIG_SERVER_PORT		5900
#define OVNC_CONFIG_CLIENT_PORT		5500

#define OVNC_CONFIG_VERSION_MAJOR	3
#define OVNC_CONFIG_VERSION_MINOR	8

#define OVNC_CONFIG_VERSION_MAJOR_MIN	3
#define OVNC_CONFIG_VERSION_MINOR_MIN	3

#define OVNC_CONFIG_VERSION_MAJOR_MAX	OVNC_CONFIG_VERSION_MAJOR
#define OVNC_CONFIG_VERSION_MINOR_MAX	OVNC_CONFIG_VERSION_MINOR


// ZLIB
#define FORCE_STATIC /*zlib*/
// avoid linking in the crc code
#define NO_GZIP


#if HAVE_CONFIG_H
	#include "../config.h"
#endif

// IMPORTANT: Do it after "config.h"
#if OVNC_UNDER_WINDOWS
#	define _WINSOCKAPI_
#	include <windows.h>
#elif OVNC_UNDER_LINUX
#elif OVNC_UNDER_APPLE
#endif

#include <stdint.h>

#endif /* OVNC_CONFIG_H */
