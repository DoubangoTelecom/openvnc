using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Input;
using KeyInput = System.Windows.Input.Key;

namespace openvncViewer.Events
{
    internal sealed class vncKeyEventArgs : vncEventArgs
    {
        private readonly bool m_bDown;
        private readonly int m_nRepeatCount;
        private readonly int m_nKey;
        private readonly bool m_bValid;

        internal struct KeyMap
        {
            internal KeyInput charpkey;
            internal int vncKey;

            internal KeyMap(KeyInput charpkey, int vncKey)
            {
                this.charpkey = charpkey;
                this.vncKey = vncKey;
            }
        }

        static KeyMap[] KeyMaps = new KeyMap[]
        {
            /* RFC 6143 - 7.5.4. KeyEvent */
            new KeyMap(KeyInput.Back, vncKeyEvent.XK_BackSpace),
            new KeyMap(KeyInput.Tab, vncKeyEvent.XK_Tab),
            new KeyMap(KeyInput.Return, vncKeyEvent.XK_Return),
            new KeyMap(KeyInput.Enter, vncKeyEvent.XK_Return),
            new KeyMap(KeyInput.Escape, vncKeyEvent.XK_Escape),
            new KeyMap(KeyInput.Insert, vncKeyEvent.XK_Insert),
            new KeyMap(KeyInput.Delete, vncKeyEvent.XK_Delete),
            new KeyMap(KeyInput.Home, vncKeyEvent.XK_Home),
            new KeyMap(KeyInput.End, vncKeyEvent.XK_End),
            new KeyMap(KeyInput.PageUp, vncKeyEvent.XK_Page_Up),
            new KeyMap(KeyInput.PageDown, vncKeyEvent.XK_Page_Down),
            new KeyMap(KeyInput.Left, vncKeyEvent.XK_Left),
            new KeyMap(KeyInput.Up, vncKeyEvent.XK_Up),
            new KeyMap(KeyInput.Right, vncKeyEvent.XK_Right),
            new KeyMap(KeyInput.Down, vncKeyEvent.XK_Down),
            new KeyMap(KeyInput.F1, vncKeyEvent.XK_F1),
            new KeyMap(KeyInput.F2, vncKeyEvent.XK_F2),
            new KeyMap(KeyInput.F3, vncKeyEvent.XK_F3),
            new KeyMap(KeyInput.F4, vncKeyEvent.XK_F4),
            new KeyMap(KeyInput.F5, vncKeyEvent.XK_F5),
            new KeyMap(KeyInput.F6, vncKeyEvent.XK_F6),
            new KeyMap(KeyInput.F7, vncKeyEvent.XK_F7),
            new KeyMap(KeyInput.F8, vncKeyEvent.XK_F8),
            new KeyMap(KeyInput.F9, vncKeyEvent.XK_F9),
            new KeyMap(KeyInput.F10, vncKeyEvent.XK_F10),
            new KeyMap(KeyInput.F11, vncKeyEvent.XK_F11),
            new KeyMap(KeyInput.F12, vncKeyEvent.XK_F12),
            new KeyMap(KeyInput.LeftShift, vncKeyEvent.XK_Shift_L),
            new KeyMap(KeyInput.RightShift, vncKeyEvent.XK_Shift_R),
            new KeyMap(KeyInput.LeftCtrl, vncKeyEvent.XK_Control_L),
            new KeyMap(KeyInput.RightCtrl, vncKeyEvent.XK_Control_R),
            new KeyMap(KeyInput.LWin, vncKeyEvent.XK_Meta_L),
            new KeyMap(KeyInput.RWin, vncKeyEvent.XK_Meta_R),
            new KeyMap(KeyInput.LeftAlt, vncKeyEvent.XK_Alt_L),
            new KeyMap(KeyInput.RightAlt, vncKeyEvent.XK_Alt_R),

            new KeyMap(KeyInput.CapsLock, vncKeyEvent.XK_Caps_Lock),
            new KeyMap(KeyInput.Capital, vncKeyEvent.XK_Shift_Lock),
            new KeyMap(KeyInput.NumLock, vncKeyEvent.XK_Num_Lock),
        };

        internal vncKeyEventArgs(KeyEventArgs e)
            :base()
        {
            m_bValid = false;
            m_bDown = e.IsDown;
            m_nRepeatCount = 1;
            
            foreach (KeyMap keyMap in KeyMaps)
            {
                if (keyMap.charpkey == e.Key)
                {
                    m_bValid = true;
                    m_nKey = keyMap.vncKey;
                    return;
                }
            }
            
            char cc = ' ';
            if (vncKeyEventArgs.ReadChar(e.Key, e.KeyboardDevice.Modifiers, ref cc))
            {
                m_bValid = true;
                m_nKey = (int)cc;
            }
        }

        internal vncKeyEventArgs(int msg, IntPtr wParam, IntPtr lParam)
            :base()
        {
            m_bValid = false;
            m_bDown = (msg == NativeMethods.WM_KEYDOWN);
            m_nRepeatCount = 1;

            switch (msg)
            {
                case NativeMethods.WM_KEYDOWN:
                case NativeMethods.WM_KEYUP:
                    {
                        Key key = KeyInterop.KeyFromVirtualKey(wParam.ToInt32());
                        
                        /*
                         * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
                         * 0x40 : unassigned
                         * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
                         */
                        if ((wParam.ToInt32() >= 0x30 && wParam.ToInt32() <= 0x39) || (wParam.ToInt32() >= 0x41 && wParam.ToInt32() <= 0x5A))
                        {
                            String keyString = new KeyConverter().ConvertToString(key);
                            if (!String.IsNullOrEmpty(keyString) && keyString.Length > 0)
                            {
                                m_bValid = true;
                                m_nKey = (int)keyString[0];
                            }
                           
                        }
                        break;
                    }
            }
        }
#if NO
        private static bool ReadChar(KeyInput key, ref char c)
        {
            int virtualKey = KeyInterop.VirtualKeyFromKey(key);
            byte[] keyboardState = new byte[256];
            uint code = NativeMethods.MapVirtualKey((uint)virtualKey, NativeMethods.MapType.MAPVK_VK_TO_VSC);
            StringBuilder sb = new StringBuilder(2);
            int result = NativeMethods.ToUnicode((uint)virtualKey, code, keyboardState, sb, sb.Capacity, 0);
            if (result != -1 && result != 0)
            {
                c = sb[0];
                return true;
            }

            return false;
        }
#endif
        private static bool ReadChar(KeyInput key, ModifierKeys modifiers, ref char c)
        {
            int virtualKey = KeyInterop.VirtualKeyFromKey(key);
            uint code = NativeMethods.MapVirtualKey((uint)virtualKey, NativeMethods.MapType.MAPVK_VK_TO_CHAR);
            if (code != 0)
            {
                if ((code >= 'A') && (code <= 'Z') && (modifiers & ModifierKeys.Shift) != ModifierKeys.Shift)
                {
                    code += 0x20;
                }
                c = (char)code;
                return true;
            }

            return false;
        }

        internal bool IsValid()
        {
            return this.Valid;
        }

        internal bool Valid
        {
            get { return m_bValid; }
        }

        internal bool IsDown()
        {
            return this.Down;
        }

        internal bool Down
        {
            get { return m_bDown; }
        }

        internal int RepeatCount
        {
            get { return m_nRepeatCount; }
        }

        internal int Key
        {
            get { return m_nKey; }
        }
    }
}
