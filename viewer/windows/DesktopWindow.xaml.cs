using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using org.doubango.openvnc;
using System.Windows.Interop;
using System.Threading;
using openvncViewer.Events;

namespace openvncViewer
{
    /// <summary>
    /// Interaction logic for DesktopWindow.xaml
    /// </summary>
    public partial class DesktopWindow : Window
    {
        private vncEngineObject m_Engine;// FIXME: Singleton global to the application
        private vncClientObject m_Client;
        private vncVideoDisplay m_VideoDisplay;
        private MyClientCallback m_Callback;

        public DesktopWindow()
        {
            InitializeComponent();
            
            m_VideoDisplay = new vncVideoDisplay();
            videoPlaceholder.Child = m_VideoDisplay;
            m_Callback = new MyClientCallback(this);

            m_VideoDisplay.Focusable = true;
            m_VideoDisplay.Focus();
        }

        ~DesktopWindow()
        {
            if (m_Client != null)
            {
                
            }
        }

        internal vncClientObject vncClient
        {
            get { return m_Client; }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            m_VideoDisplay.vncOnMouseEvent += this.VideoDisplay_vncOnMouseEvent;
            m_VideoDisplay.vncOnKeyEvent += this.VideoDisplay_vncOnKeyEvent;

            videoPlaceholder.MinWidth = 1000;

            if (m_Engine == null)
            {
                vncError_t error;

                m_Engine = vncEngine.New();
                if (m_Engine != null)
                {
                    m_Client = m_Engine.NewClient();
                }

                if (m_Client != null)
                {
                    IntPtr handle = new WindowInteropHelper(this).Handle;
                    error = m_Client.SetCallback(m_Callback);
                    //error = m_Client.SetPixelSize(vncRfbPixelSize_t.vncRfbPixelType_32);
                    error = m_Client.SetParentWindow(m_VideoDisplay.Handle.ToInt64());
                    error = m_Client.SetPassword("admin");
                    error = m_Client.Configure("192.168.0.11", 5900, vncNetworkType_t.vncNetworkType_TCP);
                    error = m_Client.Start();
                    error = m_Client.Connect();
                }
            }
        }

        private void VideoDisplay_vncOnKeyEvent(object sender, vncKeyEventArgs e)
        {
            if (m_Client != null && m_Client.IsSessionEstablished())
            {
                m_Client.SendKeyEvent(e.Down, (uint)e.Key);
            }
        }

        private void VideoDisplay_vncOnMouseEvent(object sender, vncMouseEventArgs e)
        {
            if (m_Client != null && m_Client.IsSessionEstablished())
            {
                m_Client.SendPointerEvent(e.ButtonMask, (ushort)e.X, (ushort)e.Y);
                switch (e.EventType)
                {
                    case vncMouseEventArgs.vncMouseEventType.LeftButtonDown:
                    case vncMouseEventArgs.vncMouseEventType.LeftButtonUp:
                    case vncMouseEventArgs.vncMouseEventType.RightButtonDown:
                    case vncMouseEventArgs.vncMouseEventType.RightButtonUp:
                        {
                            m_VideoDisplay.Focus();
                            break;
                        }
                }
            }
        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (m_Client != null && m_Client.IsConnected())
            {
                m_Client.Disconnect();
            }
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            if (m_Client != null)
            {
                // m_Client.SetCallback(null);
            }
        }

        private void MenuItemFullScreen_Click(object sender, RoutedEventArgs e)
        {
            menu.Visibility = Visibility.Collapsed;
            videoPlaceholder.Margin = new Thickness(0.0f, 0.0f, 0.0f, 0.0f);
            WindowStyle = WindowStyle.None;
            WindowState = WindowState.Maximized;
        }

        private void MenuItemKeys_CtrlAltDel_Click(object sender, RoutedEventArgs e)
        {
            if (m_Client != null && m_Client.IsSessionEstablished())
            {
                m_Client.SendKeyEvent(true, vncKeyEvent.XK_Control_L);
                m_Client.SendKeyEvent(true, vncKeyEvent.XK_Alt_L);
                m_Client.SendKeyEvent(true, vncKeyEvent.XK_Delete);

                m_Client.SendKeyEvent(false, vncKeyEvent.XK_Control_L);
                m_Client.SendKeyEvent(false, vncKeyEvent.XK_Alt_L);
                m_Client.SendKeyEvent(false, vncKeyEvent.XK_Delete);
            }
        }



        /// <summary>
        /// MyClientCallback
        /// </summary>
        class MyClientCallback : vncClientCallback
        {
            DesktopWindow m_Window;

            public MyClientCallback(DesktopWindow window)
            {
                m_Window = window;
            }

            public override vncError_t OnEvent(vncClientEvent pcEvent)
            {
                switch (pcEvent.GetEventType())
                {
                    case vncClientEventType_t.vncClientEventType_StateChanged:
                        {
                            vncClientState_t eState = m_Window.vncClient.GetSate();
                            switch (eState)
                            {
                                case vncClientState_t.vncClientState_Connecting:
                                default:
                                    {
                                        m_Window.Dispatcher.Invoke((System.Threading.ThreadStart)delegate
                                        {
                                            m_Window.Title = eState.ToString();
                                        });
                                        break;
                                    }
                            }
                            break;
                        }
                }
                return vncError_t.vncError_Ok;
            }
        }
    }
}
