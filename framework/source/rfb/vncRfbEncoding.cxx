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
#include "openvnc/rfb/vncRfbEncoding.h"
#include "openvnc/rfb/vncRfbEncodingZRLE.h"
#include "openvnc/vncUtils.h"
#include "openvnc/network/vncNetUtils.h"

vncRfbEncoding::vncRfbEncoding(vncRfbEncoding_t eEncoding)
:vncObject()
{
	m_eEncoding = eEncoding;
}

vncRfbEncoding::~vncRfbEncoding()
{

}

vncObjectWrapper<vncRfbEncoding*> vncRfbEncoding::New(vncRfbEncoding_t eEncoding)
{
	switch(eEncoding)
	{
		case vncRfbEncoding_ZRLE:
			{
				return new vncRfbEncodingZRLE();
			}
	}

	return NULL;
}
