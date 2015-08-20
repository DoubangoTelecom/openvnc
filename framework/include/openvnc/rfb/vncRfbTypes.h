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
#ifndef OVNC_RFB_TYPES_H
#define OVNC_RFB_TYPES_H

#include "vncConfig.h"

typedef uint32_t vncRfbPixel_t;

typedef enum vncRfbSize_e
{
	vncRfbSize_Zero = 0,
	vncRfbSize_ProtocolVersion = 12,
	vncRfbSize_SecurityResult = 4,
	vncRfbSize_VNCAuthChallenge = 16,
	vncRfbSize_SetPixelFormat = 20,
	vncRfbSize_FramebufferUpdateRequest = 10,
}
vncRfbSize_t;

typedef enum vncRfbPixelSize_e
{
	vncRfbPixelType_None,
	vncRfbPixelType_8,
	vncRfbPixelType_16,
	vncRfbPixelType_32
}
vncRfbPixelSize_t;

typedef enum vncRfbMsgbType_e
{
	vncRfbMsgType_None,
	vncRfbMsgType_ProtocolVersion,
	vncRfbMsgType_Security,
	vncRfbMsgType_VNCAuthChallenge,
	vncRfbMsgType_SecurityResult,
	
	vncRfbMsgType_ServerInit,
	vncRfbMsgType_FramebufferUpdate,
	vncRfbMsgType_SetColorMapEntries,
	vncRfbMsgType_Bell,
	vncRfbMsgType_ServerCutText,

	vncRfbMsgType_ClientInit,
	vncRfbMsgType_PointerEvent,
	vncRfbMsgType_KeyEvent,
	vncRfbMsgType_SetEncoding,
	vncRfbMsgType_SetPixelFormat,
	vncRfbMsgType_FramebufferUpdateRequest,
}
vncRfbMsgType_t;

typedef enum vncRfbServerMsg_e
{
	vncRfbServerMsg_FramebufferUpdate = 0,
	vncRfbServerMsg_SetColorMapEntries = 1,
	vncRfbServerMsg_Bell = 2,
	vncRfbServerMsg_ServerCutText = 3
};

typedef enum vncRfbClientMsg_e
{
	vncRfbClientMsg_SetPixelFormat = 0,
	vncRfbClientMsg_SetEncodings = 2,
	vncRfbClientMsg_FramebufferUpdateRequest = 3,
	vncRfbClientMsg_KeyEvent = 4,
	vncRfbClientMsg_PointerEvent = 5,
	vncRfbClientMsg_ClientCutText = 6,
	
	vncRfbClientMsg_Anthony_Liguori = 255,
	/* VMWare (257 - 127) */
	vncRfbClientMsg_gii = 253,
	vncRfbClientMsg_tight = 252,
	vncRfbClientMsg_Pierre_Ossman_SetDesktopSize = 251,
	vncRfbClientMsg_Colin_Dean_xvp = 250,
	vncRfbClientMsg_OLIVE_Call_Control = 249
}
vncRfbClientMsg_t;

typedef enum vncRfbVersion_e
{
	vncRfbVersion_None = 0,
	vncRfbVersion_3_3,
	vncRfbVersion_3_7,
	vncRfbVersion_3_8,

	// see section 6.1.1 ProtocolVersion
	vncRfbVersion_3_5 = vncRfbVersion_3_3
}
vncRfbVersion_t;

typedef enum vncRfbEncoding_e
{
	vncRfbEncoding_Raw = 0,
	vncRfbEncoding_CopyRect = 1,
	vncRfbEncoding_RRE = 2,
	vncRfbEncoding_Hextile = 5,
	vncRfbEncoding_ZRLE = 16
}
vncRfbEncoding_t;


typedef enum vncRfbSecurity_e
{
	vncRfbSecurity_Invalid = 0,
	vncRfbSecurity_None = 1,
	vncRfbSecurity_VNC_Authentication = 2,
	vncRfbSecurity_RA2 = 5,
	vncRfbSecurity_RA2ne = 6,
	vncRfbSecurity_Tight = 16,
	vncRfbSecurity_Ultra = 17,
	vncRfbSecurity_TLS = 18,
	vncRfbSecurity_VeNCrypt = 19,
	vncRfbSecurity_GTK_VNC_SASL = 20,
	vncRfbSecurity_MD5 = 21,
	vncRfbSecurity_Colin_Dean_xvp = 22
}
vncRfbSecurity_t;


typedef enum vncRfbSecurityResult_e
{
	vncRfbSecurityResult_Ok = 0,
	vncRfbSecurityResult_Failed = 1
}
vncRfbSecurityResult_t;


typedef struct vncRfbPixelFormat_s
{
	uint8_t bits_per_pixel;
	uint8_t depth;
	uint8_t big_endian_flag;
	uint8_t true_colour_flag;
	uint16_t red_max;
	uint16_t green_max;
	uint16_t blue_max;
	uint8_t red_shift;
	uint8_t green_shift;
	uint8_t blue_shift;
#if !SWIG
	uint8_t padding[3]; 
#endif
	//!\ Must not add any additional field
}
vncRfbPixelFormat_t;


typedef struct vncRfbRectangle_s
{
	//!\ Must not change order
	uint16_t x_position;
	uint16_t y_position;
	uint16_t width;
	uint16_t height;
	vncRfbEncoding_t encoding_type;
	//!\ Must not add fields
}
vncRfbRectangle_t;

typedef enum vncRfbPixelReaderType_e
{
	vncRfbPixelReaderType_C,
	vncRfbPixelReaderType_8bits,
	vncRfbPixelReaderType_16bits,
	vncRfbPixelReaderType_32bits
}
vncRfbPixelReaderType_t;

#define OVNC_RFB_RECTANGLE_HEADER_SIZE	12
#define OVNC_RFB_PIXELFORMAT_SIZE		16

#define OVNC_RFB_ZRLE_TILE_WIDTH		64
#define OVNC_RFB_ZRLE_TILE_HEIGHT		64

#endif /* OVNC_RFB_TYPES_H */
