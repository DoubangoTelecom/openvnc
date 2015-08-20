/* File : tinyWRAP.i */
%module(directors="1") OpenVNCWRAP
%include "typemaps.i"
%include <stdint.i>

%{
#include "Debug.h"
#include "vncAPI.h"
%}

%feature("director") vncClientCallback;
%feature("director") DebugCallback;

%newobject vncEngine::New();
%newobject vncEngine::NewClient();
%newobject vncClient::New();

%newobject vncEngineObject::New();
%newobject vncEngineObject::NewClient();
%newobject vncClientObject::New();

# new vncObjectWrapper<vncObject*>(const vncObjectWrapper<vncObject*>& other);
#%typemap(newfree) vncObjectWrapper<vncClient*> *{
#   delete7 $1;
#}

%nodefaultctor;
%include "Debug.h"
%include "../../../framework/include/vncConfig.h"
%include "../../../framework/include/openvnc/vncCommon.h"
%include "../../../framework/include/openvnc/vncObject.h"

%template(vncClientObject) vncObjectWrapper<vncClient*>;
%template(vncServerObject) vncObjectWrapper<vncServer*>;
%template(vncEngineObject) vncObjectWrapper<vncEngine*>;

%include "../../../framework/include/openvnc/rfb/vncRfbTypes.h"
%include "../../../framework/include/openvnc/vncRunnable.h"
%include "../../../framework/include/openvnc/vncClient.h"
%include "../../../framework/include/openvnc/vncServer.h"
%include "../../../framework/include/openvnc/vncEngine.h"
%clearnodefaultctor;
