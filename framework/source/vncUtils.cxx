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
#include "openvnc/vncUtils.h"
#include "openvnc/vncDebug.h"
extern "C"
{
	#include "openvnc/d3des.h"
}

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if OVNC_UNDER_WINDOWS
#elif OVNC_UNDER_APPLE
#	include <mach/mach.h>
#	include <mach/mach_time.h>
#else
#	include <sys/time.h>
#endif

#if defined(_MSC_VER)
#	define snprintf		_snprintf
#	define vsnprintf	_vsnprintf
#	define strdup		_strdup
#	define stricmp		_stricmp
#	define strnicmp		_strnicmp
#else
#	if !HAVE_STRNICMP && !HAVE_STRICMP
#	define stricmp		strcasecmp
#	define strnicmp		strncasecmp
#	endif
#endif

vncError_t vncUtils::IntegerToString(int64_t nNumber, vncNumberString_t *pNumber)
{
	if(!pNumber){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}

	memset(*pNumber, 0, sizeof(*pNumber));
    sprintf(*pNumber, "%lld", nNumber);

	return vncError_Success;
}

int64_t vncUtils::StringToInteger(const char* pString)
{
	if(pString){
#if defined(_MSC_VER)
		return _atoi64(pString);
#elif defined(__GNUC__)
		return atoll(pString);
#else
		return atol(pString); 
#endif
	}
	return 0;
}

int vncUtils::IndexOf(const char * pStr, unsigned nSize, const char * pSubString)
{
	if(pStr && pSubString){
		const char* pSubStringStart = strstr(pStr, pSubString);
		if(pSubStringStart && (pSubStringStart < (pStr + nSize))){
			return (pSubStringStart - pStr);
		}
	}
	return -1;
}

char* vncUtils::StringNDup(const char *str, unsigned nSize)
{
	char *ret = NULL;

	if(str && nSize){
		unsigned len = vncUtils::StringLength(str);
		unsigned nret = (nSize > len) ? (len) : (nSize);

		if((ret = (char*)vncUtils::Calloc((nret+1), sizeof(uint8_t)))){
			memcpy(ret, str, nret);
		}
	}

	return ret;
}

vncError_t vncUtils::StringUpdate(char** ppString, const char* pNewval)
{
	if(!ppString){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}
	vncUtils::Free((void**)ppString);
	*ppString = vncUtils::StringDup(pNewval);
	return vncError_Ok;
}

int vncUtils::StringPrintf(char** str, const char* format, va_list* ap)
{
	int len = 0;
    va_list ap2;

	/* free previous value */
	if(*str){
		vncUtils::Free((void**)str);
	}
	
	/* needed for 64bit platforms where vsnprintf will change the va_list */
    ovnc_va_copy(ap2, *ap);
    
	/* compute destination len for windows mobile
	*/
#if defined(_WIN32_WCE)
	{
		int n;
		len = (vncUtils::StringLength(format) * 2);
		*str = (char*)vncUtils::Calloc(1, len+1);
		for(;;){
			if( (n = vsnprintf(*str, len, format, *ap)) >= 0 && (n<len) ){
				len = n;
				goto done;
			}
			else{
				len += 10;
				*str = vncUtils::Realloc(*str, len+1);
			}
		}
done:
		(*str)[len] = '\0';
	}
#else
    len = vsnprintf(0, 0, format, *ap);
    *str = (char*)vncUtils::Calloc(1, len+1);
    vsnprintf(*str, len
#if !defined(_MSC_VER) || defined(__GNUC__)
		+1
#endif
		, format, ap2);
#endif
	
    va_end(ap2);
    
	return len;
}

//
//	Memory
//


void* vncUtils::Malloc(unsigned nSize)
{
	void *pRet = malloc(nSize);
	if(!pRet){
		OVNC_DEBUG_ERROR("Memory allocation failed");
	}
	return pRet;
}

void* vncUtils::Realloc (void * pMemory, unsigned nSize)
{
	void *pRet = NULL;
	if(pMemory){
		if(!(pRet = realloc(pMemory, nSize))){
			OVNC_DEBUG_ERROR("Memory reallocation failed");
		}
	}
	else{
		if(!(pRet = calloc(nSize, 1))){
			OVNC_DEBUG_ERROR("Memory allocation failed");
		}
	}
	return pRet;
}

void vncUtils::Free(void** ppMemory)
{
	if(ppMemory && *ppMemory){
		free(*ppMemory);
		*ppMemory = NULL;
	}
}

void* vncUtils::Calloc(unsigned nNum, unsigned nSize)
{
	void* pRet = NULL;
	if(nNum && nSize){
		pRet = calloc(nNum, nSize);
		if(!pRet){
			OVNC_DEBUG_ERROR("Memory allocation failed. num=%u and size=%u", nNum, nSize);
		}
	}
	return pRet;
}





//
//	Time
//
uint64_t vncUtils::GetNow()
{
#if OVNC_UNDER_WINDOWS
	static LARGE_INTEGER __liFrequency = {0};
	LARGE_INTEGER liPerformanceCount;
	if(!__liFrequency.QuadPart){
		QueryPerformanceFrequency(&__liFrequency);
	}
	QueryPerformanceCounter(&liPerformanceCount);
	return (uint64_t)(((double)liPerformanceCount.QuadPart/(double)__liFrequency.QuadPart)*1000.0);
#elif OVNC_HAVE_CLOCK_GETTIME || _POSIX_TIMERS > 0
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (((uint64_t)ts.tv_sec)*(uint64_t)1000) + (((uint64_t)ts.tv_nsec)/(uint64_t)1000000);
#elif OVNC_UNDER_APPLE
    static mach_timebase_info_data_t __apple_timebase_info = {0, 0};
    if (__apple_timebase_info.denom == 0) {
        (void) mach_timebase_info(&__apple_timebase_info);
    }
    return (uint64_t)((mach_absolute_time() * __apple_timebase_info.numer) / (1e+6 * __apple_timebase_info.denom));
#else
	struct timeval tv;
	gettimeofday(&tv, 0); 
	return (((uint64_t)tv.tv_sec)*(uint64_t)1000) + (((uint64_t)tv.tv_usec)/(uint64_t)1000);
#endif
}



vncError_t vncUtils::D3DesEncrypt(const char* pPassword, const char aData[16], char aResult[16], unsigned &nResultSize)
{
	if(!pPassword){
		OVNC_DEBUG_ERROR("Invalid parameter");
		return vncError_InvalidParameter;
	}	
	
	nResultSize = 16;

	char key[16];
	memset(key, 0, sizeof(key));
	memcpy(key, pPassword, OVNC_MIN(StringLength(pPassword), 8));

	deskey((unsigned char*)key, EN0);
	des((unsigned char*)aData, (unsigned char*)aResult);
	des((unsigned char*)aData + 8, (unsigned char*)aResult + 8);
	return vncError_Success;
}
