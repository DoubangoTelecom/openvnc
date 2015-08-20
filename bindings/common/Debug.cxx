#include "Debug.h"
#include "openvnc/vncDebug.h"
#include "openvnc/vncUtils.h"

#if ANDROID /* callbacks will fail with jni */
#	include <android/log.h>
#	define ANDROID_DEBUG_TAG "openvncWRAP"
#endif

/* Very Important ==> never call functions which could raise debug callbacks into callback functions
*  Callbacks should not used with Android (JNI).
*/

enum cb_type{
	cb_info,
	cb_warn,
	cb_error,
	cb_fatal
};

int debug_xxx_cb(const void* arg, const char* fmt, enum cb_type type, va_list *app)
{
	int ret = -1;
	if(!arg){
		return -1;
	}

	const DebugCallback* callback = dynamic_cast<const DebugCallback*>((const DebugCallback*)arg);

 	if(callback){
		char* message = NULL;		
		vncUtils::StringPrintf(&message, fmt, app);
		
		switch(type){
			case cb_info:
				ret=
#if ANDROID
				__android_log_write(ANDROID_LOG_INFO, ANDROID_DEBUG_TAG, message);
#else
				callback->OnDebugInfo(message);
#endif
				break;
			case cb_warn:
				ret=
#if ANDROID
				__android_log_write(ANDROID_LOG_WARN, ANDROID_DEBUG_TAG, message);
#else
				callback->OnDebugWarn(message);
#endif
				break;
			case cb_error:
				ret=
#if ANDROID
				__android_log_write(ANDROID_LOG_ERROR, ANDROID_DEBUG_TAG, message);
#else
				callback->OnDebugError(message);
#endif
				break;
			case cb_fatal:
				ret=
#if ANDROID
				__android_log_write(ANDROID_LOG_FATAL, ANDROID_DEBUG_TAG, message);
#else
				callback->OnDebugFatal(message);
#endif
				break;
		}
		
		OVNC_SAFE_FREE(message);
	}

	return ret;
}

int DebugCallback::debug_info_cb(const void* arg, const char* fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = debug_xxx_cb(arg, fmt, cb_info, &ap);
	va_end(ap);
	
	return ret;
}

int DebugCallback::debug_warn_cb(const void* arg, const char* fmt, ...){
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = debug_xxx_cb(arg, fmt, cb_warn, &ap);
	va_end(ap);

	return ret;
}

int DebugCallback::debug_error_cb(const void* arg, const char* fmt, ...){
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = debug_xxx_cb(arg, fmt, cb_error, &ap);
	va_end(ap);

	return ret;
}

int DebugCallback::debug_fatal_cb(const void* arg, const char* fmt, ...){
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = debug_xxx_cb(arg, fmt, cb_fatal, &ap);
	va_end(ap);

	return ret;
}


DebugCallback::DebugCallback() 
{  
	ovnc_debug_set_arg_data(this);
	ovnc_debug_set_info_cb(DebugCallback::debug_info_cb);
	ovnc_debug_set_warn_cb(DebugCallback::debug_warn_cb);
	ovnc_debug_set_error_cb(DebugCallback::debug_error_cb);
	ovnc_debug_set_fatal_cb(DebugCallback::debug_fatal_cb);
}

DebugCallback::~DebugCallback() 
{
	ovnc_debug_set_arg_data(NULL);
	ovnc_debug_set_info_cb(NULL);
	ovnc_debug_set_warn_cb(NULL);
	ovnc_debug_set_error_cb(NULL);
	ovnc_debug_set_fatal_cb(NULL);
}