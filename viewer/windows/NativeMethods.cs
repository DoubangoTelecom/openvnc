using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace openvncViewer
{
    internal static class NativeMethods
    {
        internal const int GWL_WNDPROC = -4;

        internal const int WM_PAINT = 0x000F;
        internal const int WM_SIZE = 0x0005;
        internal const int WM_SETREDRAW = 0x000B;
        internal const int WM_USER = 0x400;
        internal const int WM_NCHITTEST = 0x0084;

        internal const int WM_MOUSEFIRST = 0x0200;
        internal const int WM_MOUSEMOVE = 0x0200;
        internal const int WM_LBUTTONDOWN = 0x0201;
        internal const int WM_LBUTTONUP = 0x0202;
        internal const int WM_LBUTTONDBLCLK = 0x0203;
        internal const int WM_RBUTTONDOWN = 0x0204;
        internal const int WM_RBUTTONUP = 0x0205;
        internal const int WM_RBUTTONDBLCLK = 0x0206;
        internal const int WM_MBUTTONDOWN = 0x0207;
        internal const int WM_MBUTTONUP = 0x0208;
        internal const int WM_MBUTTONDBLCLK = 0x0209;

        internal const int WM_KEYFIRST = 0x0100;
        internal const int WM_KEYDOWN = 0x0100;
        internal const int WM_KEYUP = 0x0101;
        internal const int WM_CHAR = 0x0102;
        internal const int WM_DEADCHAR = 0x0103;
        internal const int WM_SYSKEYDOWN = 0x0104;
        internal const int WM_SYSKEYUP = 0x0105;
        internal const int WM_SYSCHAR = 0x0106;
        internal const int WM_SYSDEADCHAR = 0x0107;

        internal const int MK_CONTROL = 0x0008;
        internal const int MK_LBUTTON = 0x0001;
        internal const int MK_MBUTTON = 0x0010;
        internal const int MK_RBUTTON = 0x0002;
        internal const int MK_SHIFT = 0x0004;
        internal const int MK_XBUTTON1 = 0x0020;
        internal const int MK_XBUTTON2 = 0x0040;

        
        internal const int  VK_LBUTTON = 0x01;
        internal const int  VK_RBUTTON = 0x02;
        internal const int  VK_CANCEL = 0x03;
        internal const int  VK_MBUTTON = 0x04;    /* NOT contiguous with L & RBUTTON */

        internal const int  VK_XBUTTON1 = 0x05;    /* NOT contiguous with L & RBUTTON */
        internal const int  VK_XBUTTON2 = 0x06;    /* NOT contiguous with L & RBUTTON */

        internal const int  VK_BACK = 0x08;
        internal const int  VK_TAB = 0x09;

        internal const int  VK_CLEAR = 0x0C;
        internal const int  VK_RETURN = 0x0D;

        internal const int  VK_SHIFT = 0x10;
        internal const int  VK_CONTROL = 0x11;
        internal const int  VK_MENU = 0x12;
        internal const int  VK_PAUSE = 0x13;
        internal const int  VK_CAPITAL = 0x14;

        internal const int  VK_KANA = 0x15;
        internal const int  VK_HANGEUL = 0x15;  /* old name - should be here for compatibility */
        internal const int  VK_HANGUL = 0x15;
        internal const int  VK_JUNJA = 0x17;
        internal const int  VK_FINAL = 0x18;
        internal const int  VK_HANJA = 0x19;
        internal const int  VK_KANJI = 0x19;

        internal const int  VK_ESCAPE = 0x1B;

        internal const int  VK_CONVERT = 0x1C;
        internal const int  VK_NONCONVERT = 0x1D;
        internal const int  VK_ACCEPT = 0x1E;
        internal const int  VK_MODECHANGE = 0x1F;

        internal const int  VK_SPACE = 0x20;
        internal const int  VK_PRIOR = 0x21;
        internal const int  VK_NEXT = 0x22;
        internal const int  VK_END = 0x23;
        internal const int  VK_HOME = 0x24;
        internal const int  VK_LEFT = 0x25;
        internal const int  VK_UP = 0x26;
        internal const int  VK_RIGHT = 0x27;
        internal const int  VK_DOWN = 0x28;
        internal const int  VK_SELECT = 0x29;
        internal const int  VK_PRINT = 0x2A;
        internal const int  VK_EXECUTE = 0x2B;
        internal const int  VK_SNAPSHOT = 0x2C;
        internal const int  VK_INSERT = 0x2D;
        internal const int  VK_DELETE = 0x2E;
        internal const int  VK_HELP = 0x2F;

        /*
         * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
         * 0x40 : unassigned
         * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
         */
        internal const int  VK_LWIN = 0x5B;
        internal const int  VK_RWIN = 0x5C;
        internal const int  VK_APPS = 0x5D;

        internal const int  VK_SLEEP = 0x5F;

        internal const int  VK_NUMPAD0 = 0x60;
        internal const int  VK_NUMPAD1 = 0x61;
        internal const int  VK_NUMPAD2 = 0x62;
        internal const int  VK_NUMPAD3 = 0x63;
        internal const int  VK_NUMPAD4 = 0x64;
        internal const int  VK_NUMPAD5 = 0x65;
        internal const int  VK_NUMPAD6 = 0x66;
        internal const int  VK_NUMPAD7 = 0x67;
        internal const int  VK_NUMPAD8 = 0x68;
        internal const int  VK_NUMPAD9 = 0x69;
        internal const int  VK_MULTIPLY = 0x6A;
        internal const int  VK_ADD = 0x6B;
        internal const int  VK_SEPARATOR = 0x6C;
        internal const int  VK_SUBTRACT = 0x6D;
        internal const int  VK_DECIMAL = 0x6E;
        internal const int  VK_DIVIDE = 0x6F;
        internal const int  VK_F1 = 0x70;
        internal const int  VK_F2 = 0x71;
        internal const int  VK_F3 = 0x72;
        internal const int  VK_F4 = 0x73;
        internal const int  VK_F5 = 0x74;
        internal const int  VK_F6 = 0x75;
        internal const int  VK_F7 = 0x76;
        internal const int  VK_F8 = 0x77;
        internal const int  VK_F9 = 0x78;
        internal const int  VK_F10 = 0x79;
        internal const int  VK_F11 = 0x7A;
        internal const int  VK_F12 = 0x7B;
        internal const int  VK_F13 = 0x7C;
        internal const int  VK_F14 = 0x7D;
        internal const int  VK_F15 = 0x7E;
        internal const int  VK_F16 = 0x7F;
        internal const int  VK_F17 = 0x80;
        internal const int  VK_F18 = 0x81;
        internal const int  VK_F19 = 0x82;
        internal const int  VK_F20 = 0x83;
        internal const int  VK_F21 = 0x84;
        internal const int  VK_F22 = 0x85;
        internal const int  VK_F23 = 0x86;
        internal const int  VK_F24 = 0x87;

        internal const int  VK_NUMLOCK = 0x90;
        internal const int VK_SCROLL = 0x91;

        internal const int WS_CHILD = 0x40000000;
        internal const int WS_VISIBLE = 0x10000000;
        internal const int WS_CLIPSIBLINGS = 0x04000000;
        internal const int WS_CLIPCHILDREN = 0x02000000;

        internal delegate IntPtr WndProc(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll", EntryPoint = "DestroyWindow", CharSet = CharSet.Auto)]
        internal static extern bool DestroyWindow(IntPtr hwnd);

        [DllImport("user32.dll", EntryPoint = "CreateWindowEx", CharSet = CharSet.Auto)]
        internal static extern IntPtr CreateWindowEx(int dwExStyle,
            string lpszClassName,
            string lpszWindowName,
            int style,
            int x, int y,
            int width, int height,
            IntPtr hwndParent,
            IntPtr hMenu,
            IntPtr hInst,
            [MarshalAs(UnmanagedType.AsAny)] object pvParam);


        [DllImport("user32.dll")]
        internal static extern ushort RegisterClass([In] ref WNDCLASS lpWndClass);

        [DllImport("user32.dll")]
        internal static extern long SendMessageA(IntPtr hWnd, int wMsg, IntPtr wParam, IntPtr lParam);
        
        [DllImport("user32", CharSet = CharSet.Auto)]
        internal static extern IntPtr SendMessage(IntPtr hWnd, int msg, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool SetWindowPos(IntPtr hWnd, Int32 hWndInsertAfter, Int32 X, Int32 Y, Int32 cx, Int32 cy, uint uFlags);

        [DllImport("user32")]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool ShowScrollBar(IntPtr handle, int wBar, [MarshalAs(UnmanagedType.Bool)] bool bShow);

        [DllImport("user32.dll")]
        internal static extern int HideCaret(IntPtr hwnd);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto)]
        internal static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        internal static extern int AnimateWindow(IntPtr hwand, int dwTime, int dwFlags);

        [DllImport("user32.dll")]
        internal extern static IntPtr SetWindowLong(IntPtr hwnd, int nIndex, IntPtr dwNewLong);

        [DllImport("user32.dll")]
        internal extern static int ToUnicode(uint wVirtKey,uint wScanCode,byte[] lpKeyState,[Out, MarshalAs(UnmanagedType.LPWStr, SizeParamIndex = 4)] StringBuilder pwszBuff,int cchBuff,uint wFlags);

        [DllImport("user32.dll")]
        internal extern static bool GetKeyboardState(byte[] lpKeyState);

        [DllImport("user32.dll")]
        internal extern static uint MapVirtualKey(uint uCode, MapType uMapType);


        internal static ushort LOWORD(IntPtr l)
        {
            return (ushort)(((long)l) & 0xffff);
        }
        internal static ushort LOWORD(uint l)
        {
            return (ushort)(l & 0xffff);
        }
        internal static ushort HIWORD(IntPtr l)
        {
            return (ushort)((((long)l) >> 0x10) & 0xffff);
        }
        internal static ushort HIWORD(uint l)
        {
            return (ushort)(l >> 0x10);
        }
        internal static int GET_Y_LPARAM(IntPtr lParam)
        {
            return (short)HIWORD(lParam);
        }
        internal static int GET_X_LPARAM(IntPtr lParam)
        {
            return (short)LOWORD(lParam);
        }

        internal enum MapType : uint
        {
            MAPVK_VK_TO_VSC = 0x0,
            MAPVK_VSC_TO_VK = 0x1,
            MAPVK_VK_TO_CHAR = 0x2,
            MAPVK_VSC_TO_VK_EX = 0x3,
        }

        [Flags]
        internal enum ClassStyles : uint
        {
            ByteAlignClient = 0x1000,
            ByteAlignWindow = 0x2000,
            ClassDC = 0x40,
            DoubleClicks = 0x8,
            DropShadow = 0x20000,
            GlobalClass = 0x4000,
            HorizontalRedraw = 0x2,
            NoClose = 0x200,
            OwnDC = 0x20,
            ParentDC = 0x80,
            SaveBits = 0x800,
            VerticalRedraw = 0x1
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct WNDCLASS
        {
            internal ClassStyles style;
            [MarshalAs(UnmanagedType.FunctionPtr)]
            internal WndProc lpfnWndProc;
            internal int cbClsExtra;
            internal int cbWndExtra;
            internal IntPtr hInstance;
            internal IntPtr hIcon;
            internal IntPtr hCursor;
            internal IntPtr hbrBackground;
            [MarshalAs(UnmanagedType.LPTStr)]
            internal string lpszMenuName;
            [MarshalAs(UnmanagedType.LPTStr)]
            internal string lpszClassName;
        }
    }
}
