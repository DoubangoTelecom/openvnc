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
using System.Windows.Navigation;
using System.Windows.Shapes;
using org.doubango.openvnc;
using System.Windows.Interop;

namespace openvncViewer
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        
        private vncDebugCallback m_DebugCallback;

        public Window1()
        {
            InitializeComponent();

            m_DebugCallback = new vncDebugCallback();
        }


        class vncDebugCallback : DebugCallback
        {
            public override int OnDebugInfo(string message)
            {
                Console.WriteLine(message);
                return 0;
            }

            public override int OnDebugWarn(string message)
            {
                Console.WriteLine(message);
                return 0;
            }

            public override int OnDebugError(string message)
            {
                Console.WriteLine(message);
                return 0;
            }

            public override int OnDebugFatal(string message)
            {
                Console.WriteLine(message);
                return 0;
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            
        }

        private void button1_Click(object sender, RoutedEventArgs e)
        {
            DesktopWindow desktopWindow = new DesktopWindow();
            desktopWindow.Show();
        }
    }
}
