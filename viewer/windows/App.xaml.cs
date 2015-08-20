using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;
using org.doubango.openvnc;

namespace openvncViewer
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private void Application_Startup(object sender, StartupEventArgs e)
        {
            if (vncEngine.Initialize() != vncError_t.vncError_Ok)
            {
                MessageBox.Show("Failed Initialize the vncEngine");
                Shutdown();
            }
        }
    }
}
