/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.39
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace org.doubango.openvnc {

using System;
using System.Runtime.InteropServices;

public class vncClientEvent : vncObject {
  private HandleRef swigCPtr;

  internal vncClientEvent(IntPtr cPtr, bool cMemoryOwn) : base(OpenVNCWRAPPINVOKE.vncClientEventUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(vncClientEvent obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~vncClientEvent() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        OpenVNCWRAPPINVOKE.delete_vncClientEvent(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public vncClientEvent(vncClientEventType_t eType) : this(OpenVNCWRAPPINVOKE.new_vncClientEvent((int)eType), true) {
  }

  public virtual string GetObjectId() {
    string ret = OpenVNCWRAPPINVOKE.vncClientEvent_GetObjectId(swigCPtr);
    return ret;
  }

  public virtual vncClientEventType_t GetEventType() {
    vncClientEventType_t ret = (vncClientEventType_t)OpenVNCWRAPPINVOKE.vncClientEvent_GetEventType(swigCPtr);
    return ret;
  }

}

}