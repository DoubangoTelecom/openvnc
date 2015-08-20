using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace openvncViewer.Events
{
    internal sealed class vncStringEventArgs : vncEventArgs
    {
        private readonly String m_Value;

        internal vncStringEventArgs(String value)
            :base()
        {
            m_Value = value;
        }

        internal String Value
        {
            get { return m_Value; }
        }
    }
}
