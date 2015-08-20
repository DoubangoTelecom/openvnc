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
#ifndef OVNC_OBJECT_H
#define OVNC_OBJECT_H

#include "vncDebug.h"

#define vncObjectSafeRelease(pObject)	(pObject) = NULL
#define vncObjectSafeFree				vncObjectSafeRelease

class OVNC_API vncObject
{
protected:
	vncObject();
	vncObject(const vncObject &);

public:
	virtual ~vncObject();
#if !SWIG
	virtual OVNC_INLINE const char* GetObjectId() = 0;
	OVNC_INLINE int GetRefCount() const{
		return m_nRefCount;
	}
	void operator=(const vncObject &);
#endif


public:
#if !SWIG
	int TakeRef() const;
	int ReleaseRef() const;
#endif

private:
	mutable int	m_nRefCount;
};


//
//	vncObjectWrapper declaration
//
template<class vncObjectType>
class vncObjectWrapper{

public:
	OVNC_INLINE vncObjectWrapper(vncObjectType obj = NULL);
	OVNC_INLINE vncObjectWrapper(const vncObjectWrapper<vncObjectType> &obj);
	OVNC_INLINE virtual ~vncObjectWrapper();

public:
#if SWIG
	%rename(opEquals) operator=;
	%rename(opIsEquals) operator==;
	%rename(opNotEquals) operator!=;
	%rename(opLessThan) operator<;
	%rename(opDotPtr) operator->;
	%rename(opMul) operator*;
	%rename(opBool) operator bool;
#endif
	OVNC_INLINE vncObjectWrapper<vncObjectType>& operator=(const vncObjectType other);
	OVNC_INLINE vncObjectWrapper<vncObjectType>& operator=(const vncObjectWrapper<vncObjectType> &other);
	OVNC_INLINE bool operator ==(const vncObjectWrapper<vncObjectType> other) const;
	OVNC_INLINE bool operator!=(const vncObjectWrapper<vncObjectType> &other) const;
	OVNC_INLINE bool operator <(const vncObjectWrapper<vncObjectType> other) const;
	OVNC_INLINE vncObjectType operator->() const;
	OVNC_INLINE vncObjectType operator*() const;
	OVNC_INLINE operator bool() const;

protected:
	OVNC_INLINE int TakeRef();
	OVNC_INLINE int ReleaseRef();

	OVNC_INLINE vncObjectType GetWrappedObject() const;
	OVNC_INLINE void WrapObject(vncObjectType obj);

private:
	vncObjectType m_WrappedObject;
};

//
//	vncObjectWrapper implementation
//
template<class vncObjectType>
vncObjectWrapper<vncObjectType>::vncObjectWrapper(vncObjectType obj) { 
	WrapObject(obj), TakeRef();
}

template<class vncObjectType>
vncObjectWrapper<vncObjectType>::vncObjectWrapper(const vncObjectWrapper<vncObjectType> &obj) {
	WrapObject(obj.GetWrappedObject()),
	TakeRef();
}

template<class vncObjectType>
vncObjectWrapper<vncObjectType>::~vncObjectWrapper(){
	ReleaseRef(),
	WrapObject(NULL);
}


template<class vncObjectType>
int vncObjectWrapper<vncObjectType>::TakeRef(){
	if(m_WrappedObject /*&& m_WrappedObject->getRefCount() At startup*/){
		return m_WrappedObject->TakeRef();
	}
	return 0;
}

template<class vncObjectType>
int vncObjectWrapper<vncObjectType>::ReleaseRef() {
	if(m_WrappedObject && m_WrappedObject->GetRefCount()){
		if(m_WrappedObject->ReleaseRef() == 0){
			delete m_WrappedObject, m_WrappedObject = NULL;
		}
		else{
			return m_WrappedObject->GetRefCount();
		}
	}
	return 0;
}

template<class vncObjectType>
vncObjectType vncObjectWrapper<vncObjectType>::GetWrappedObject() const{
	return m_WrappedObject;
}

template<class vncObjectType>
void vncObjectWrapper<vncObjectType>::WrapObject(const vncObjectType obj){
	if(obj){
		if(!(m_WrappedObject = dynamic_cast<vncObjectType>(obj))){
			OVNC_DEBUG_ERROR("Trying to wrap an object with an invalid type");
		}
	}
	else{
		m_WrappedObject = NULL;
	}
}

template<class vncObjectType>
vncObjectWrapper<vncObjectType>& vncObjectWrapper<vncObjectType>::operator=(const vncObjectType obj){
	ReleaseRef();
	WrapObject(obj), TakeRef();
	return *this;
}

template<class vncObjectType>
vncObjectWrapper<vncObjectType>& vncObjectWrapper<vncObjectType>::operator=(const vncObjectWrapper<vncObjectType> &other){
	ReleaseRef();
	WrapObject(other.GetWrappedObject()), TakeRef();
	return *this;
}


template<class vncObjectType>
bool vncObjectWrapper<vncObjectType>::operator ==(const vncObjectWrapper<vncObjectType> other) const {
	return GetWrappedObject() == other.GetWrappedObject();
}

template<class vncObjectType>
bool vncObjectWrapper<vncObjectType>::operator!=(const vncObjectWrapper<vncObjectType> &other) const {
	return GetWrappedObject() != other.GetWrappedObject();
}

template<class vncObjectType>
bool vncObjectWrapper<vncObjectType>::operator <(const vncObjectWrapper<vncObjectType> other) const {
	return GetWrappedObject() < other.GetWrappedObject();
}

template<class vncObjectType>
vncObjectWrapper<vncObjectType>::operator bool() const {
	return (GetWrappedObject() != NULL);
}

template<class vncObjectType>
vncObjectType vncObjectWrapper<vncObjectType>::operator->() const {
	return GetWrappedObject();
}

template<class vncObjectType>
vncObjectType vncObjectWrapper<vncObjectType>::operator*() const{
	return GetWrappedObject();
}


#endif /* OVNC_OBJECT_H */
