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
#ifndef OVNC_UTILS_H
#define OVNC_UTILS_H

#include "vncConfig.h"
#include "openvnc/vncCommon.h"


#define vncUtils_SystemPrintLastError(fmt, ...) \
	{ \
		vncNetError_t error; \
		vncUtils::SystemGetLastError(&error); \
		OVNC_DEBUG_ERROR(fmt, ##__VA_ARGS__); \
		OVNC_DEBUG_ERROR("(SYSTEM) ERROR ==>%s", error) \
	}

class OVNC_API vncUtils
{
public:
	//
	//	System
	//
	static OVNC_INLINE int SystemGetErrno()
	{
	#if OVNC_UNDER_WINDOWS
		// return WSAGetLastError();
		return GetLastError();
	#else
		return errno;
	#endif
	}

	static OVNC_INLINE void SystemGetLastError(vncSystemError_t *pError)
	{
		int err  = vncUtils::SystemGetErrno();
		memset(*pError, 0, sizeof(*pError));

	#if OVNC_UNDER_WINDOWS
		{
	#ifdef _WIN32_WCE
			FormatMessage
	#else
			FormatMessageA
	#endif
			(
			  FORMAT_MESSAGE_FROM_SYSTEM, 
			  NULL,
			  err,
			  0,
			  *pError, 
			  sizeof(*pError)-1,
			  NULL);
		}
	#else
		strerror_r(err, *pError, sizeof(*pError));
	#endif
	}

	//
	//	String
	//
	static vncError_t IntegerToString(int64_t nNumber, vncNumberString_t *pNumber);
	static int64_t StringToInteger(const char* pString);
	static int IndexOf(const char * pStr, unsigned nSize, const char * pSubString);
	static OVNC_INLINE bool StringIsNullEmpty(const char *str){ return !str || *str == '\0'; }
	static OVNC_INLINE unsigned StringLength(const char *str){ return str ? (unsigned)strlen(str) : 0; }
	static OVNC_INLINE char* StringDup(const char* str){ return str ? strdup(str) : NULL; }
	static char* StringNDup(const char *str, unsigned nSize);
	static vncError_t StringUpdate(char** ppString, const char* pNewval);
	static int StringPrintf(char** str, const char* format, va_list* ap);

	//
	//	Memory
	//
	static void* Malloc(unsigned nSize);
	static void* Realloc (void * pMemory, unsigned nSize);
	static void Free(void** ppMemory);
	static void* Calloc(unsigned nNum, unsigned nSize);

	//
	//	Time
	//
	static uint64_t GetNow();

	//
	//	Encryption
	//
	static vncError_t D3DesEncrypt(const char* pPassword, const char aData[16], char aResult[16], unsigned &nResultSize);
};

#endif /* OVNC_UTILS_H */
