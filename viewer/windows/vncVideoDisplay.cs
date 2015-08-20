using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Interop;
using System.Runtime.InteropServices;
using openvncViewer.Events;

namespace openvncViewer
{
    internal sealed class vncVideoDisplay : HwndHost
    {
        public event EventHandler<vncMouseEventArgs> vncOnMouseEvent;
        public event EventHandler<vncKeyEventArgs> vncOnKeyEvent;

        private IntPtr m_hWnd;

        protected override HandleRef BuildWindowCore(HandleRef hwndParent)
        {
            int width = (int)this.Width;
            int height  = (int)this.Height;

            m_hWnd = NativeMethods.CreateWindowEx(0, "vncVideoDisplay", String.Empty,
                NativeMethods.WS_CHILD | NativeMethods.WS_VISIBLE | NativeMethods.WS_CLIPSIBLINGS | NativeMethods.WS_CLIPCHILDREN,
                0, 0,
                width, height,
                hwndParent.Handle,
                IntPtr.Zero,
                IntPtr.Zero,
                0);

            return new HandleRef(this, this.m_hWnd);
        }

        protected override void OnKeyDown(System.Windows.Input.KeyEventArgs e)
        {
            if (!e.Handled)
            {
                vncKeyEventArgs eArgs = new vncKeyEventArgs(e);
                if (eArgs.IsValid() && this.vncOnKeyEvent != null)
                {
                    vncEventHandlerTrigger.TriggerEvent<vncKeyEventArgs>(this.vncOnKeyEvent, this, eArgs);
                }
            }
            e.Handled = true;
            base.OnKeyUp(e);
        }

        protected override void OnKeyUp(System.Windows.Input.KeyEventArgs e)
        {
            if (!e.Handled)
            {
                vncKeyEventArgs eArgs = new vncKeyEventArgs(e);
                if (eArgs.IsValid() && this.vncOnKeyEvent != null)
                {
                    vncEventHandlerTrigger.TriggerEvent<vncKeyEventArgs>(this.vncOnKeyEvent, this, eArgs);
                }
            }
            e.Handled = true;
            base.OnKeyUp(e);
        }

        protected override void OnMouseMove(System.Windows.Input.MouseEventArgs e)
        {
            base.OnMouseMove(e);
        }

        protected override void OnWindowPositionChanged(System.Windows.Rect rcBoundingBox)
        {
            base.OnWindowPositionChanged(rcBoundingBox);
        }

        protected override void DestroyWindowCore(HandleRef hwnd)
        {
            NativeMethods.DestroyWindow(hwnd.Handle);
        }

        protected override IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            vncMouseEventArgs.vncMouseEventType eventType = vncMouseEventArgs.vncMouseEventType.None;
            switch (msg)
            {
                case NativeMethods.WM_MOUSEMOVE:
                    {
                        eventType = openvncViewer.Events.vncMouseEventArgs.vncMouseEventType.Move;
                        break;
                    }
                case NativeMethods.WM_LBUTTONDOWN:
                    {
                        eventType = openvncViewer.Events.vncMouseEventArgs.vncMouseEventType.LeftButtonDown;
                        break;
                    }
                case NativeMethods.WM_LBUTTONUP:
                    {
                        eventType = openvncViewer.Events.vncMouseEventArgs.vncMouseEventType.LeftButtonUp;
                        break;
                    }
                case NativeMethods.WM_RBUTTONDOWN:
                    {
                        eventType = openvncViewer.Events.vncMouseEventArgs.vncMouseEventType.RightButtonDown;
                        break;
                    }
                case NativeMethods.WM_RBUTTONUP:
                    {
                        eventType = openvncViewer.Events.vncMouseEventArgs.vncMouseEventType.RightButtonUp;
                        break;
                    }

                //case NativeMethods.WM_KEYDOWN:
                //case NativeMethods.WM_KEYUP:
                //    {
                //        vncKeyEventArgs eArgs = new vncKeyEventArgs(msg, wParam, lParam);
                //        if (eArgs.IsValid() && this.vncOnKeyEvent != null)
                //        {
                //            vncEventHandlerTrigger.TriggerEvent<vncKeyEventArgs>(this.vncOnKeyEvent, this, eArgs);
                //        }
                //        break;
                //    }
            }

            if (eventType != vncMouseEventArgs.vncMouseEventType.None && this.vncOnMouseEvent != null)
            {
                vncEventHandlerTrigger.TriggerEvent<vncMouseEventArgs>(this.vncOnMouseEvent, this,
                            new vncMouseEventArgs(eventType, wParam, lParam));
            }

            return base.WndProc(hwnd, msg, wParam, lParam, ref handled);
        }
    }

}
