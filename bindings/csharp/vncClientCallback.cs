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

public class vncClientCallback : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal vncClientCallback(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(vncClientCallback obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~vncClientCallback() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        OpenVNCWRAPPINVOKE.delete_vncClientCallback(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public vncClientCallback() : this(OpenVNCWRAPPINVOKE.new_vncClientCallback(), true) {
    SwigDirectorConnect();
  }

  public virtual vncError_t OnEvent(vncClientEvent pcEvent) {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientCallback_OnEvent(swigCPtr, vncClientEvent.getCPtr(pcEvent));
    return ret;
  }

  private void SwigDirectorConnect() {
    if (SwigDerivedClassHasMethod("OnEvent", swigMethodTypes0))
      swigDelegate0 = new SwigDelegatevncClientCallback_0(SwigDirectorOnEvent);
    OpenVNCWRAPPINVOKE.vncClientCallback_director_connect(swigCPtr, swigDelegate0);
  }

  private bool SwigDerivedClassHasMethod(string methodName, Type[] methodTypes) {
    System.Reflection.MethodInfo methodInfo = this.GetType().GetMethod(methodName, System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance, null, methodTypes, null);
    bool hasDerivedMethod = methodInfo.DeclaringType.IsSubclassOf(typeof(vncClientCallback));
    return hasDerivedMethod;
  }

  private int SwigDirectorOnEvent(IntPtr pcEvent) {
    return (int)OnEvent((pcEvent == IntPtr.Zero) ? null : new vncClientEvent(pcEvent, false));
  }

  public delegate int SwigDelegatevncClientCallback_0(IntPtr pcEvent);

  private SwigDelegatevncClientCallback_0 swigDelegate0;

  private static Type[] swigMethodTypes0 = new Type[] { typeof(vncClientEvent) };
}

}
