#ifndef OVNC_WRAP_DEBUG_H
#define OVNC_WRAP_DEBUG_H

#include "openvncWRAPConfig.h"

class DebugCallback
{
public:
	DebugCallback();
	virtual ~DebugCallback();

public:
	virtual int OnDebugInfo(const char* message)const { return -1; }
	virtual int OnDebugWarn(const char* message)const { return -1; }
	virtual int OnDebugError(const char* message)const { return -1; }
	virtual int OnDebugFatal(const char* message)const { return -1; }
	
#if !defined(SWIG)
public:
	static int debug_info_cb(const void* arg, const char* fmt, ...);
	static int debug_warn_cb(const void* arg, const char* fmt, ...);
	static int debug_error_cb(const void* arg, const char* fmt, ...);
	static int debug_fatal_cb(const void* arg, const char* fmt, ...);
#endif

private:
	
};

#endif /* OVNC_WRAP_DEBUG_H */
