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
#ifndef OVNC_RFB_MSG_H
#define OVNC_RFB_MSG_H

#include "vncConfig.h"
#include "openvnc/vncObject.h"
#include "openvnc/vncCommon.h"
#include "openvnc/rfb/vncRfbTypes.h"
#include "openvnc/vncBuffer.h"
#include "openvnc/vncUtils.h"
#include "openvnc/rfb/vncRfbRectangleData.h"

#include <list>

class vncMsg : public vncObject
{
protected:
	vncMsg(vncRfbMsgType_t eType);
public:
	virtual ~vncMsg();
	virtual OVNC_INLINE vncRfbMsgType_t GetType(){ return m_eType; }
	virtual OVNC_INLINE unsigned GetSize() = 0;
	virtual vncObjectWrapper<vncBuffer*> GetBytes() = 0;

	static vncObjectWrapper<vncMsg*> ParseServerMsg(const void* pData, unsigned nDataSize, vncError_t &eResult);
	static vncObjectWrapper<vncMsg*> ParseClientMsg(const void* pData, unsigned nDataSize, vncError_t &eResult);

protected:
	vncRfbMsgType_t m_eType;
};


class vncMsgProtocolVersion : public vncMsg
{
protected:
	vncMsgProtocolVersion(int nMajorVersion = OVNC_CONFIG_VERSION_MAJOR, int nMinorVersion = OVNC_CONFIG_VERSION_MINOR);
public:
	virtual ~vncMsgProtocolVersion();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncMsgProtocolVersion"; }
	virtual OVNC_INLINE unsigned GetSize(){ return (unsigned)vncRfbSize_ProtocolVersion; }
	virtual vncObjectWrapper<vncBuffer*> GetBytes();
	virtual OVNC_INLINE int GetMajorVersion(){ return m_nMajorVersion; }
	virtual OVNC_INLINE int GetMinorVersion(){ return m_nMinorVersion; }

	static vncObjectWrapper<vncMsgProtocolVersion*> Parse(const void* pData, unsigned nDataSize, vncError_t &eResult);

private:
	int m_nMajorVersion;
	int m_nMinorVersion;
};


class vncMsgSecurity : public vncMsg
{
public:
	vncMsgSecurity();
	virtual ~vncMsgSecurity();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncMsgSecurity"; }
	virtual OVNC_INLINE std::list<vncRfbSecurity_t>* GetSecurities(){ return &m_aSecurity; }
	virtual OVNC_INLINE unsigned GetSize(){ return m_aSecurity.size() + 1/* number-of-security-types */; }
	virtual void AddSecurity(vncRfbSecurity_t eSecurity){ 
		m_aSecurity.push_back(eSecurity);
	}
	virtual vncObjectWrapper<vncBuffer*> GetBytes();

	static vncObjectWrapper<vncMsgSecurity*> Parse(const void* pData, unsigned nDataSize, vncError_t &eResult);

private:
	std::list<vncRfbSecurity_t> m_aSecurity;
};


class vncMsgSecurityResult : public vncMsg
{
protected:
	vncMsgSecurityResult();
public:
	virtual ~vncMsgSecurityResult();

public:
	virtual OVNC_INLINE const char* GetObjectId() { return "vncMsgSecurityResult"; }
	virtual OVNC_INLINE unsigned GetSize(){ return (unsigned)vncRfbSize_SecurityResult; }
	virtual vncObjectWrapper<vncBuffer*> GetBytes();
	virtual OVNC_INLINE vncRfbSecurityResult_t GetResult(){ return m_eResult; }
	
	static vncObjectWrapper<vncMsgSecurityResult*> Parse(const void* pData, unsigned nDataSize, vncError_t &eResult);

private:
	vncRfbSecurityResult_t m_eResult;
};

class vncMsgVNCAuthChallenge : public vncMsg
{
protected:
	vncMsgVNCAuthChallenge();
public:
	virtual ~vncMsgVNCAuthChallenge();

public:
	virtual OVNC_INLINE const char* GetObjectId() { return "vncMsgVNCAuthChallenge"; }
	virtual OVNC_INLINE unsigned GetSize(){ return (unsigned)vncRfbSize_VNCAuthChallenge; }
	virtual vncObjectWrapper<vncBuffer*> GetBytes();
	virtual OVNC_INLINE const char* GetResponse(const char* password);
	virtual OVNC_INLINE unsigned GetResponseSize(){ return sizeof(m_aResponse); }

	static vncObjectWrapper<vncMsgVNCAuthChallenge*> Parse(const void* pData, unsigned nDataSize, vncError_t &eResult);

private:
	char m_aChallenge[16];
	char m_aResponse[16];
	bool m_bResponseComputed;
};


class vncMsgServerInit : public vncMsg
{
public:
	vncMsgServerInit();
	virtual ~vncMsgServerInit();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncMsgServerInit"; }
	virtual OVNC_INLINE unsigned GetSize(){ return 2 + 2 + 16 + 4 + m_nNameLength; }
	virtual OVNC_INLINE uint16_t GetWidth(){ return m_nWidth; }
	virtual OVNC_INLINE uint16_t GetHeight(){ return m_nHeight; }
	virtual OVNC_INLINE void SetWidth(uint16_t nWidth){ m_nWidth = nWidth; }
	virtual OVNC_INLINE void SetHeight(uint16_t nHeight){ m_nHeight = nHeight; }
	virtual OVNC_INLINE vncRfbPixelFormat_t* GetPixelFormat(){ return &m_PixelFormat; }
	virtual vncError_t SetPixelFormat(const vncRfbPixelFormat_t* pFormat){
		if(!pFormat){
			OVNC_DEBUG_ERROR("Invalid parameter");
			return vncError_InvalidParameter;
		}
		memcpy(&m_PixelFormat, pFormat, sizeof(vncRfbPixelFormat_t));
		return vncError_Ok;
	}
	virtual OVNC_INLINE unsigned GetPixelFormatSize(){ return sizeof(vncRfbPixelFormat_t); }
	virtual OVNC_INLINE unsigned GetNameLength(){ return m_nNameLength; }
	virtual OVNC_INLINE uint8_t* GetName(){ return m_pName; }
	virtual vncError_t SetName(const char* pName){
		if(!pName){
			OVNC_DEBUG_ERROR("Invalid parameter");
			return vncError_InvalidParameter;
		}
		vncError_t ret = vncUtils::StringUpdate((char**)&m_pName, pName);
		if(ret == vncError_Ok){
			m_nNameLength = vncUtils::StringLength((const char*)m_pName);
		}
		return ret;
	}
	virtual vncObjectWrapper<vncBuffer*> GetBytes();

	static vncObjectWrapper<vncMsgServerInit*> Parse(const void* pData, unsigned nDataSize, vncError_t &eResult);

private:
	uint16_t m_nWidth;
	uint16_t m_nHeight;
	vncRfbPixelFormat_t m_PixelFormat;
	uint32_t m_nNameLength;
	uint8_t* m_pName;
};

class vncMsgSetEncoding : public vncMsg
{
public:
	vncMsgSetEncoding();
	virtual ~vncMsgSetEncoding();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncMsgSetEncoding"; }
	virtual OVNC_INLINE unsigned GetSize(){ 
		return  
			1 /* Message Type */ +
			1 /* Padding */ +
			2 /* Number-of-encodings */ +
			m_aEncodings.size() * 4;
	}
	virtual vncObjectWrapper<vncBuffer*> GetBytes();
	virtual void AddEncoding(vncRfbEncoding_t eEncoding){ m_aEncodings.push_back(eEncoding); }
	virtual OVNC_INLINE const std::list<vncRfbEncoding_t>* GetEncodings(){ return &m_aEncodings; }

	static vncObjectWrapper<vncMsgSetEncoding*> Parse(const void* pData, unsigned nDataSize, vncError_t &eResult);

private:
	std::list<vncRfbEncoding_t>m_aEncodings;
};




class vncMsgSetPixelFormat : public vncMsg
{
public:
	vncMsgSetPixelFormat();
	virtual ~vncMsgSetPixelFormat();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncMsgSetPixelFormat"; }
	virtual OVNC_INLINE unsigned GetSize(){ 
		return  
			1 /* Message Type */ +
			3 /* Padding */ +
			OVNC_RFB_PIXELFORMAT_SIZE /* pixel-format */;
	}
	virtual vncError_t SetFormat(const vncRfbPixelFormat_t* pFormat);
	virtual OVNC_INLINE const vncRfbPixelFormat_t* GetFormat(){ return &m_PixelFormat; }
	virtual vncObjectWrapper<vncBuffer*> GetBytes();

	static vncObjectWrapper<vncMsgSetPixelFormat*> Parse(const void* pData, unsigned nDataSize, vncError_t &eResult);

private:
	vncRfbPixelFormat_t m_PixelFormat;
};




class vncMsgFramebufferUpdateRequest : public vncMsg
{
public:
	vncMsgFramebufferUpdateRequest();
	virtual ~vncMsgFramebufferUpdateRequest();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncMsgFramebufferUpdateRequest"; }
	virtual OVNC_INLINE unsigned GetSize(){ 
		return  
			1 /* message-type */ +
			1 /* incremental */ +
			2 /* x-position */ +
			2 /* y-position */ +
			2 /* width */ +
			2 /* height */;
	}
	virtual vncObjectWrapper<vncBuffer*> GetBytes();
	virtual OVNC_INLINE void SetIncremental(bool bIncremental){ m_bIncremental = bIncremental; }
	virtual OVNC_INLINE bool IsIncremental(){ return m_bIncremental; }
	virtual OVNC_INLINE void SetPosition(uint16_t nX, uint16_t nY){ m_nXPosition = nX; m_nYPosition = nY; }
	virtual OVNC_INLINE uint16_t GetXPosition(){ return m_nXPosition; }
	virtual OVNC_INLINE uint16_t GetYPosition(){ return m_nYPosition; }
	virtual OVNC_INLINE void SetSize(uint16_t nWidth, uint16_t nHeight){ m_nWidth = nWidth; m_nHeight = nHeight; }
	virtual OVNC_INLINE uint16_t GetWidth(){ return m_nWidth; }
	virtual OVNC_INLINE uint16_t GetHeight(){ return m_nHeight; }

	static vncObjectWrapper<vncMsgFramebufferUpdateRequest*> Parse(const void* pData, unsigned nDataSize, vncError_t &eResult);
	
private:
	bool m_bIncremental;
	uint16_t m_nXPosition;
	uint16_t m_nYPosition;
	uint16_t m_nWidth;
	uint16_t m_nHeight;
};



class vncMsgFramebufferUpdate : public vncMsg
{
public:
	vncMsgFramebufferUpdate();
	virtual ~vncMsgFramebufferUpdate();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncMsgFramebufferUpdate"; }
	virtual OVNC_INLINE unsigned GetSize(){ return m_nSize; }
	virtual vncObjectWrapper<vncBuffer*> GetBytes();
	virtual vncError_t AddRectangle(vncRfbRectangle_t* pRectangle, const void* pData, unsigned nDataSize);
	virtual OVNC_INLINE std::list<vncObjectWrapper<vncRfbRectangleData*>>* GetRectangles(){ return &m_Rectangles; }

	static vncObjectWrapper<vncMsgFramebufferUpdate*> Parse(const void* pData, unsigned nDataSize, vncError_t &eResult);

private:
	std::list<vncObjectWrapper<vncRfbRectangleData*>>m_Rectangles;
	unsigned m_nSize;
};





class vncMsgPointerEvent : public vncMsg
{
public:
	vncMsgPointerEvent(uint8_t nButtonMask = 0, uint16_t nXPosition = 0, uint16_t nYPosition = 0);
	virtual ~vncMsgPointerEvent();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncMsgPointerEvent"; }
	virtual OVNC_INLINE unsigned GetSize(){ 
		return 
			1 /* message-type */ +
			1 /* button-mask */ +
			2 /* x-position */ +
			2 /* y-position*/;
	}
	virtual vncObjectWrapper<vncBuffer*> GetBytes();
	virtual OVNC_INLINE void SetButtonMask(uint8_t nButtonMask){ m_nButtonMask = nButtonMask; }
	virtual OVNC_INLINE void SetXPosition(uint16_t nXPosition){ m_nXPosition = nXPosition; }
	virtual OVNC_INLINE void SetYPosition(uint16_t nYPosition){ m_nYPosition = nYPosition; }
	virtual OVNC_INLINE void SetPosition(uint16_t nXPosition, uint16_t nYPosition){ 
		SetXPosition(nXPosition);
		SetYPosition(nYPosition); 
	}

	static vncObjectWrapper<vncMsgPointerEvent*> Parse(const void* pData, unsigned nDataSize, vncError_t &eResult);

private:
	uint8_t m_nButtonMask;
	uint16_t m_nXPosition;
	uint16_t m_nYPosition;
};


class vncMsgKeyEvent : public vncMsg
{
public:
	vncMsgKeyEvent(bool bDownFlag = false, uint32_t nKey = 0);
	virtual ~vncMsgKeyEvent();
	virtual OVNC_INLINE const char* GetObjectId() { return "vncMsgKeyEvent"; }
	virtual OVNC_INLINE unsigned GetSize(){ 
		return 
			1 /* message-type */ +
			1 /* down-flag */ +
			2 /* padding */ +
			4 /* key*/;
	}
	virtual vncObjectWrapper<vncBuffer*> GetBytes();
	virtual OVNC_INLINE void SetDownFlag(bool bDownFlag){ m_bDownFlag = bDownFlag; }
	virtual OVNC_INLINE void SetKey(uint32_t nKey){ m_nKey = nKey; }

	static vncObjectWrapper<vncMsgKeyEvent*> Parse(const void* pData, unsigned nDataSize, vncError_t &eResult);

private:
	bool m_bDownFlag;
	uint32_t m_nKey;
};



#endif /* OVNC_RFB_MSG_H */
