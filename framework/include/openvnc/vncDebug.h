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
#ifndef OVNC_DEBUG_H
#define OVNC_DEBUG_H

#include "vncConfig.h"

#include <stdio.h>

#if !defined(OVNC_DEBUG_LEVEL)
#	define OVNC_DEBUG_LEVEL OVNC_DEBUG_LEVEL_ERROR
#endif

#define OVNC_DEBUG_LEVEL_INFO		4
#define OVNC_DEBUG_LEVEL_WARN		3
#define OVNC_DEBUG_LEVEL_ERROR		2
#define OVNC_DEBUG_LEVEL_FATAL		1

#if OVNC_HAVE_DEBUG_H
#	include <ovnc_debug.h>
#else
	typedef int (*ovnc_debug_f)(const void* arg, const char* fmt, ...);

	/* INFO */
#	if (OVNC_DEBUG_LEVEL >= OVNC_DEBUG_LEVEL_INFO)
#		define OVNC_DEBUG_INFO(FMT, ...)		\
			if(ovnc_debug_get_info_cb()) \
				ovnc_debug_get_info_cb()(ovnc_debug_get_arg_data(), "*INFO: " FMT "\n", ##__VA_ARGS__); \
			else \
				fprintf(stderr, "*INFO: " FMT "\n", ##__VA_ARGS__);
#	else
#		define OVNC_DEBUG_INFO(FMT, ...)		((void)0)
#	endif
	/* WARN */
#	if (DEBUG_LEVEL >= DEBUG_LEVEL_WARN)
#		define OVNC_DEBUG_WARN(FMT, ...)		\
			if(ovnc_debug_get_warn_cb()) \
				ovnc_debug_get_warn_cb()(ovnc_debug_get_arg_data(), "**WARN: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
			else \
				fprintf(stderr, "**WARN: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__);
#	else
#		define OVNC_DEBUG_WARN(FMT, ...)		((void)0)
#	endif
	/* ERROR */
#	if (DEBUG_LEVEL >= DEBUG_LEVEL_ERROR)
#		define OVNC_DEBUG_ERROR(FMT, ...) 		\
			if(ovnc_debug_get_error_cb()) \
				ovnc_debug_get_error_cb()(ovnc_debug_get_arg_data(), "***ERROR: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
			else \
				fprintf(stderr, "***ERROR: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); 
#	else
#		define OVNC_DEBUG_ERROR(FMT, ...)		((void)0)
#	endif
	/* FATAL */
#	if (DEBUG_LEVEL >= DEBUG_LEVEL_FATAL)
#		define OVNC_DEBUG_FATAL(FMT, ...)		\
			if(ovnc_debug_get_fatal_cb()) \
				ovnc_debug_get_fatal_cb()(ovnc_debug_get_arg_data(), "****FATAL: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__); \
			else \
				fprintf(stderr, "****FATAL: function: \"%s()\" \nfile: \"%s\" \nline: \"%u\" \nMSG: " FMT "\n", __FUNCTION__,  __FILE__, __LINE__, ##__VA_ARGS__);
#	else
#		define OVNC_DEBUG_FATAL(FMT, ...)		((void)0)
#	endif

static const void* ovnc_debug_arg_data = NULL;
static ovnc_debug_f ovnc_debug_info_cb = NULL;
static ovnc_debug_f ovnc_debug_warn_cb = NULL;
static ovnc_debug_f ovnc_debug_error_cb = NULL;
static ovnc_debug_f ovnc_debug_fatal_cb = NULL;

OVNC_API OVNC_INLINE void ovnc_debug_set_arg_data(const void* arg_data){
	ovnc_debug_arg_data = arg_data;
}
OVNC_API OVNC_INLINE const void* ovnc_debug_get_arg_data(){
	return ovnc_debug_arg_data;
}
OVNC_API OVNC_INLINE void ovnc_debug_set_info_cb(ovnc_debug_f cb){
	ovnc_debug_info_cb = cb;
}
OVNC_API OVNC_INLINE ovnc_debug_f ovnc_debug_get_info_cb(){
	return ovnc_debug_info_cb;
}
OVNC_API OVNC_INLINE void ovnc_debug_set_warn_cb(ovnc_debug_f cb){
	ovnc_debug_warn_cb = cb;
}
OVNC_API OVNC_INLINE ovnc_debug_f ovnc_debug_get_warn_cb(){
	return ovnc_debug_warn_cb;
}
OVNC_API OVNC_INLINE void ovnc_debug_set_error_cb(ovnc_debug_f cb){
	ovnc_debug_error_cb = cb;
}
OVNC_API OVNC_INLINE ovnc_debug_f ovnc_debug_get_error_cb(){
	return ovnc_debug_error_cb;
}
OVNC_API OVNC_INLINE void ovnc_debug_set_fatal_cb(ovnc_debug_f cb){
	ovnc_debug_fatal_cb = cb;
}
OVNC_API OVNC_INLINE ovnc_debug_f ovnc_debug_get_fatal_cb(){
	return ovnc_debug_fatal_cb;
}

#endif /* OVNC_HAVE_DEBUG_H */

#endif /* OVNC_DEBUG_H */
