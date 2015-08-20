using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace openvncViewer.Events
{
    internal sealed class vncMouseEventArgs : vncEventArgs
    {
        private readonly int m_X;
        private readonly int m_Y;
        private readonly vncMouseEventType m_Type;
        private readonly byte m_ButtonMask;

        internal enum vncMouseEventType
        {
            None,

            Move,
            LeftButtonDown,
            LeftButtonUp,
            RightButtonDown,
            RightButtonUp,
        }

        internal vncMouseEventArgs(vncMouseEventType eventType, IntPtr wParam, IntPtr lParam)
            :base()
        {
            m_Type = eventType;
            m_ButtonMask = 0;
            m_X = 0;
            m_Y = 0;

            // Mouse Position
            switch (m_Type)
            {
                case vncMouseEventType.LeftButtonDown:
                case vncMouseEventType.LeftButtonUp:
                case vncMouseEventType.Move:
                case vncMouseEventType.RightButtonDown:
                case vncMouseEventType.RightButtonUp:
                    {
                        m_X = NativeMethods.GET_X_LPARAM(lParam);
                        m_Y = NativeMethods.GET_Y_LPARAM(lParam);

                        if((wParam.ToInt32() & NativeMethods.MK_LBUTTON) == NativeMethods.MK_LBUTTON)
                        {
                            m_ButtonMask |= 0x01;
                        }
                        if ((wParam.ToInt32() & NativeMethods.MK_MBUTTON) == NativeMethods.MK_MBUTTON)
                        {
                            m_ButtonMask |= 0x02;
                        }
                        if ((wParam.ToInt32() & NativeMethods.MK_RBUTTON) == NativeMethods.MK_RBUTTON)
                        {
                            m_ButtonMask |= 0x04;
                        }
                        break;
                    }
            }
        }

        internal vncMouseEventType EventType
        {
            get { return m_Type; }
        }

        internal int X
        {
            get { return m_X; }
        }

        internal int Y
        {
            get { return m_Y; }
        }

        internal byte ButtonMask
        {
            get { return m_ButtonMask; }
        }
    }
}
