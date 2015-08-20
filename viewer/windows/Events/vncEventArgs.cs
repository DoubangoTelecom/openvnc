using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace openvncViewer.Events
{
    internal class vncEventArgs : EventArgs
    {
        private readonly IDictionary<String, Object> m_Extras;

        public vncEventArgs()
            :base()
        {
            this.m_Extras = new Dictionary<String, Object>();
        }

        internal Object GetExtra(String key)
        {
            if (this.m_Extras.ContainsKey(key))
            {
                return this.m_Extras[key];
            }
            return null;
        }

        internal vncEventArgs AddExtra(String key, Object value)
        {
            if (!this.m_Extras.ContainsKey(key))
            {
                this.m_Extras.Add(key, value);
            }
            return this;
        }
    }
}
