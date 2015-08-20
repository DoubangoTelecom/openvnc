using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace openvncViewer
{
    internal static class vncKeyEvent
    {
        internal const int XK_BackSpace = 0xFF08;	/* back space, back char */
        internal const int XK_Tab = 0xFF09;
        internal const int XK_Linefeed = 0xFF0A;	/* Linefeed, LF */
        internal const int XK_Clear = 0xFF0B;
        internal const int XK_Return = 0xFF0D;	/* Return, enter */
        internal const int XK_Pause = 0xFF13;	/* Pause, hold */
        internal const int XK_Scroll_Lock = 0xFF14;
        internal const int XK_Sys_Req = 0xFF15;
        internal const int XK_Escape = 0xFF1B;
        internal const int XK_Delete = 0xFFFF;	/* Delete, rubout */


        internal const int  XK_Home = 0xFF50;
        internal const int  XK_Left = 0xFF51;	/* Move left, left arrow */
        internal const int  XK_Up = 0xFF52;	/* Move up, up arrow */
        internal const int  XK_Right = 0xFF53;	/* Move right, right arrow */
        internal const int  XK_Down = 0xFF54;	/* Move down, down arrow */
        internal const int  XK_Prior = 0xFF55;	/* Prior, previous */
        internal const int  XK_Page_Up = 0xFF55;
        internal const int  XK_Next = 0xFF56;	/* Next */
        internal const int  XK_Page_Down = 0xFF56;
        internal const int  XK_End = 0xFF57;	/* EOL */
        internal const int  XK_Begin = 0xFF58;	/* BOL */


        /* Misc Functions */

        internal const int  XK_Select = 0xFF60;	/* Select, mark */
        internal const int  XK_Print = 0xFF61;
        internal const int  XK_Execute = 0xFF62;	/* Execute, run, do */
        internal const int  XK_Insert = 0xFF63;	/* Insert, insert here */
        internal const int  XK_Undo = 0xFF65;	/* Undo, oops */
        internal const int  XK_Redo = 0xFF66;	/* redo, again */
        internal const int  XK_Menu = 0xFF67;
        internal const int  XK_Find = 0xFF68;	/* Find, search */
        internal const int  XK_Cancel = 0xFF69;	/* Cancel, stop, abort, exit */
        internal const int  XK_Help = 0xFF6A;	/* Help */
        internal const int  XK_Break = 0xFF6B;
        internal const int  XK_Mode_switch = 0xFF7E;	/* Character set switch */
        internal const int  XK_script_switch = 0xFF7E;  /* Alias for mode_switch */
        internal const int  XK_Num_Lock = 0xFF7F;

        /*
         * Auxilliary Functions; note the duplicate definitions for left and right
         * function keys;  Sun keyboards and a few other manufactures have such
         * function key groups on the left and/or right sides of the keyboard.
         * We've not found a keyboard with more than 35 function keys total.
         */

        internal const int  XK_F1 = 0xFFBE;
        internal const int  XK_F2 = 0xFFBF;
        internal const int  XK_F3 = 0xFFC0;
        internal const int  XK_F4 = 0xFFC1;
        internal const int  XK_F5 = 0xFFC2;
        internal const int  XK_F6 = 0xFFC3;
        internal const int  XK_F7 = 0xFFC4;
        internal const int  XK_F8 = 0xFFC5;
        internal const int  XK_F9 = 0xFFC6;
        internal const int  XK_F10 = 0xFFC7;
        internal const int  XK_F11 = 0xFFC8;
        internal const int  XK_L1 = 0xFFC8;
        internal const int  XK_F12 = 0xFFC9;
        internal const int  XK_L2 = 0xFFC9;
        internal const int  XK_F13 = 0xFFCA;
        internal const int  XK_L3 = 0xFFCA;
        internal const int  XK_F14 = 0xFFCB;
        internal const int  XK_L4 = 0xFFCB;
        internal const int  XK_F15 = 0xFFCC;
        internal const int  XK_L5 = 0xFFCC;
        internal const int  XK_F16 = 0xFFCD;
        internal const int  XK_L6 = 0xFFCD;
        internal const int  XK_F17 = 0xFFCE;
        internal const int  XK_L7 = 0xFFCE;
        internal const int  XK_F18 = 0xFFCF;
        internal const int  XK_L8 = 0xFFCF;
        internal const int  XK_F19 = 0xFFD0;
        internal const int  XK_L9 = 0xFFD0;
        internal const int  XK_F20 = 0xFFD1;
        internal const int  XK_L10 = 0xFFD1;
        internal const int  XK_F21 = 0xFFD2;
        internal const int  XK_R1 = 0xFFD2;
        internal const int  XK_F22 = 0xFFD3;
        internal const int  XK_R2 = 0xFFD3;
        internal const int  XK_F23 = 0xFFD4;
        internal const int  XK_R3 = 0xFFD4;
        internal const int  XK_F24 = 0xFFD5;
        internal const int  XK_R4 = 0xFFD5;
        internal const int  XK_F25 = 0xFFD6;
        internal const int  XK_R5 = 0xFFD6;
        internal const int  XK_F26 = 0xFFD7;
        internal const int  XK_R6 = 0xFFD7;
        internal const int  XK_F27 = 0xFFD8;
        internal const int  XK_R7 = 0xFFD8;
        internal const int  XK_F28 = 0xFFD9;
        internal const int  XK_R8 = 0xFFD9;
        internal const int  XK_F29 = 0xFFDA;
        internal const int XK_R9 = 0xFFDA;
        internal const int  XK_F30 = 0xFFDB;
        internal const int  XK_R10 = 0xFFDB;
        internal const int  XK_F31 = 0xFFDC;
        internal const int  XK_R11 = 0xFFDC;
        internal const int  XK_F32 = 0xFFDD;
        internal const int  XK_R12 = 0xFFDD;
        internal const int  XK_F33 = 0xFFDE;
        internal const int  XK_R13 = 0xFFDE;
        internal const int  XK_F34 = 0xFFDF;
        internal const int  XK_R14 = 0xFFDF;
        internal const int  XK_F35 = 0xFFE0;
        internal const int  XK_R15 = 0xFFE0;


        /* Modifiers */

        internal const int  XK_Shift_L = 0xFFE1;	/* Left shift */
        internal const int  XK_Shift_R = 0xFFE2;	/* Right shift */
        internal const int  XK_Control_L = 0xFFE3;	/* Left control */
        internal const int  XK_Control_R = 0xFFE4;	/* Right control */
        internal const int  XK_Caps_Lock = 0xFFE5;	/* Caps lock */
        internal const int  XK_Shift_Lock = 0xFFE6;	/* Shift lock */

        internal const int  XK_Meta_L = 0xFFE7;	/* Left meta */
        internal const int  XK_Meta_R = 0xFFE8;	/* Right meta */
        internal const int  XK_Alt_L = 0xFFE9;	/* Left alt */
        internal const int  XK_Alt_R = 0xFFEA;	/* Right alt */
        internal const int  XK_Super_L = 0xFFEB;	/* Left super */
        internal const int  XK_Super_R = 0xFFEC;	/* Right super */
        internal const int  XK_Hyper_L = 0xFFED;	/* Left hyper */
        internal const int  XK_Hyper_R = 0xFFEE;	/* Right hyper */
    }
}
