/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.39
 * 
 * This file is not intended to be easily readable and contains a number of 
 * coding conventions designed to improve portability and efficiency. Do not make
 * changes to this file unless you know what you are doing--modify the SWIG 
 * interface file instead. 
 * ----------------------------------------------------------------------------- */

#ifndef SWIG_OpenVNCWRAP_WRAP_H_
#define SWIG_OpenVNCWRAP_WRAP_H_

class SwigDirector_DebugCallback : public DebugCallback, public Swig::Director {

public:
    SwigDirector_DebugCallback();
    virtual ~SwigDirector_DebugCallback();
    virtual int OnDebugInfo(char const *message) const;
    virtual int OnDebugWarn(char const *message) const;
    virtual int OnDebugError(char const *message) const;
    virtual int OnDebugFatal(char const *message) const;

    typedef int (SWIGSTDCALL* SWIG_Callback0_t)(char *);
    typedef int (SWIGSTDCALL* SWIG_Callback1_t)(char *);
    typedef int (SWIGSTDCALL* SWIG_Callback2_t)(char *);
    typedef int (SWIGSTDCALL* SWIG_Callback3_t)(char *);
    void swig_connect_director(SWIG_Callback0_t callbackOnDebugInfo, SWIG_Callback1_t callbackOnDebugWarn, SWIG_Callback2_t callbackOnDebugError, SWIG_Callback3_t callbackOnDebugFatal);

private:
    SWIG_Callback0_t swig_callbackOnDebugInfo;
    SWIG_Callback1_t swig_callbackOnDebugWarn;
    SWIG_Callback2_t swig_callbackOnDebugError;
    SWIG_Callback3_t swig_callbackOnDebugFatal;
    void swig_init_callbacks();
};

class SwigDirector_vncClientCallback : public vncClientCallback, public Swig::Director {

public:
    SwigDirector_vncClientCallback();
    virtual ~SwigDirector_vncClientCallback();
    virtual vncError_t OnEvent(vncClientEvent const *pcEvent) const;

    typedef int (SWIGSTDCALL* SWIG_Callback0_t)(void *);
    void swig_connect_director(SWIG_Callback0_t callbackOnEvent);

private:
    SWIG_Callback0_t swig_callbackOnEvent;
    void swig_init_callbacks();
};


#endif
