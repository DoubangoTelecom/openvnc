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

public class vncClientObject : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal vncClientObject(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(vncClientObject obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~vncClientObject() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero && swigCMemOwn) {
        swigCMemOwn = false;
        OpenVNCWRAPPINVOKE.delete_vncClientObject(swigCPtr);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }

  public vncClientObject(vncClient obj) : this(OpenVNCWRAPPINVOKE.new_vncClientObject__SWIG_0(vncClient.getCPtr(obj)), true) {
  }

  public vncClientObject() : this(OpenVNCWRAPPINVOKE.new_vncClientObject__SWIG_1(), true) {
  }

  public vncClientObject(vncClientObject obj) : this(OpenVNCWRAPPINVOKE.new_vncClientObject__SWIG_2(vncClientObject.getCPtr(obj)), true) {
    if (OpenVNCWRAPPINVOKE.SWIGPendingException.Pending) throw OpenVNCWRAPPINVOKE.SWIGPendingException.Retrieve();
  }

  public vncClientObject opEquals(vncClient other) {
    vncClientObject ret = new vncClientObject(OpenVNCWRAPPINVOKE.vncClientObject_opEquals__SWIG_0(swigCPtr, vncClient.getCPtr(other)), false);
    return ret;
  }

  public vncClientObject opEquals(vncClientObject other) {
    vncClientObject ret = new vncClientObject(OpenVNCWRAPPINVOKE.vncClientObject_opEquals__SWIG_1(swigCPtr, vncClientObject.getCPtr(other)), false);
    if (OpenVNCWRAPPINVOKE.SWIGPendingException.Pending) throw OpenVNCWRAPPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool opIsEquals(vncClientObject other) {
    bool ret = OpenVNCWRAPPINVOKE.vncClientObject_opIsEquals(swigCPtr, vncClientObject.getCPtr(other));
    if (OpenVNCWRAPPINVOKE.SWIGPendingException.Pending) throw OpenVNCWRAPPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool opNotEquals(vncClientObject other) {
    bool ret = OpenVNCWRAPPINVOKE.vncClientObject_opNotEquals(swigCPtr, vncClientObject.getCPtr(other));
    if (OpenVNCWRAPPINVOKE.SWIGPendingException.Pending) throw OpenVNCWRAPPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public bool opLessThan(vncClientObject other) {
    bool ret = OpenVNCWRAPPINVOKE.vncClientObject_opLessThan(swigCPtr, vncClientObject.getCPtr(other));
    if (OpenVNCWRAPPINVOKE.SWIGPendingException.Pending) throw OpenVNCWRAPPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public vncClient __deref__() {
    IntPtr cPtr = OpenVNCWRAPPINVOKE.vncClientObject___deref__(swigCPtr);
    vncClient ret = (cPtr == IntPtr.Zero) ? null : new vncClient(cPtr, false);
    return ret;
  }

  public vncClient __ref__() {
    IntPtr cPtr = OpenVNCWRAPPINVOKE.vncClientObject___ref__(swigCPtr);
    vncClient ret = (cPtr == IntPtr.Zero) ? null : new vncClient(cPtr, false);
    return ret;
  }

  public bool opBool() {
    bool ret = OpenVNCWRAPPINVOKE.vncClientObject_opBool(swigCPtr);
    return ret;
  }

  public string GetObjectId() {
    string ret = OpenVNCWRAPPINVOKE.vncClientObject_GetObjectId(swigCPtr);
    return ret;
  }

  public bool IsValid() {
    bool ret = OpenVNCWRAPPINVOKE.vncClientObject_IsValid(swigCPtr);
    return ret;
  }

  public vncClientState_t GetSate() {
    vncClientState_t ret = (vncClientState_t)OpenVNCWRAPPINVOKE.vncClientObject_GetSate(swigCPtr);
    return ret;
  }

  public vncError_t SetPassword(string pPassword) {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_SetPassword(swigCPtr, pPassword);
    return ret;
  }

  public vncError_t SetPixelSize(vncRfbPixelSize_t ePixelSize) {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_SetPixelSize(swigCPtr, (int)ePixelSize);
    return ret;
  }

  public vncError_t SetPixelFormat(vncRfbPixelFormat_t pPixelFormat) {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_SetPixelFormat(swigCPtr, vncRfbPixelFormat_t.getCPtr(pPixelFormat));
    return ret;
  }

  public vncError_t Configure(string pServerHost, ushort nServerPort, vncNetworkType_t eType, vncNetIPVersion_t eIPVersion) {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_Configure__SWIG_0(swigCPtr, pServerHost, nServerPort, (int)eType, (int)eIPVersion);
    return ret;
  }

  public vncError_t Configure(string pServerHost, ushort nServerPort, vncNetworkType_t eType) {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_Configure__SWIG_1(swigCPtr, pServerHost, nServerPort, (int)eType);
    return ret;
  }

  public vncError_t Start() {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_Start(swigCPtr);
    return ret;
  }

  public vncError_t Connect() {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_Connect(swigCPtr);
    return ret;
  }

  public vncError_t Disconnect() {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_Disconnect(swigCPtr);
    return ret;
  }

  public bool IsConnected() {
    bool ret = OpenVNCWRAPPINVOKE.vncClientObject_IsConnected(swigCPtr);
    return ret;
  }

  public bool IsSessionEstablished() {
    bool ret = OpenVNCWRAPPINVOKE.vncClientObject_IsSessionEstablished(swigCPtr);
    return ret;
  }

  public vncError_t SetParentWindow(byte[] pParentWindow) {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_SetParentWindow__SWIG_0(swigCPtr, pParentWindow);
    return ret;
  }

  public vncError_t SetParentWindow(long parentWindow) {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_SetParentWindow__SWIG_1(swigCPtr, parentWindow);
    return ret;
  }

  public vncError_t SetCallback(vncClientCallback pcCallback) {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_SetCallback(swigCPtr, vncClientCallback.getCPtr(pcCallback));
    return ret;
  }

  public vncError_t SendPointerEvent(byte nButtonMask, ushort nXPosition, ushort nYPosition) {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_SendPointerEvent(swigCPtr, nButtonMask, nXPosition, nYPosition);
    return ret;
  }

  public vncError_t SendKeyEvent(bool bDownFlag, uint nKey) {
    vncError_t ret = (vncError_t)OpenVNCWRAPPINVOKE.vncClientObject_SendKeyEvent(swigCPtr, bDownFlag, nKey);
    return ret;
  }

  public vncClientObject New(string pLocalHost, ushort nLocalPort, vncNetworkType_t eType) {
    vncClientObject ret = new vncClientObject(OpenVNCWRAPPINVOKE.vncClientObject_New__SWIG_0(swigCPtr, pLocalHost, nLocalPort, (int)eType), true);
    return ret;
  }

  public vncClientObject New(string pLocalHost, ushort nLocalPort) {
    vncClientObject ret = new vncClientObject(OpenVNCWRAPPINVOKE.vncClientObject_New__SWIG_1(swigCPtr, pLocalHost, nLocalPort), true);
    return ret;
  }

  public vncClientObject New(string pLocalHost) {
    vncClientObject ret = new vncClientObject(OpenVNCWRAPPINVOKE.vncClientObject_New__SWIG_2(swigCPtr, pLocalHost), true);
    return ret;
  }

  public vncClientObject New() {
    vncClientObject ret = new vncClientObject(OpenVNCWRAPPINVOKE.vncClientObject_New__SWIG_3(swigCPtr), true);
    return ret;
  }

  public vncClient ABCD() {
    IntPtr cPtr = OpenVNCWRAPPINVOKE.vncClientObject_ABCD(swigCPtr);
    vncClient ret = (cPtr == IntPtr.Zero) ? null : new vncClient(cPtr, false);
    return ret;
  }

}

}