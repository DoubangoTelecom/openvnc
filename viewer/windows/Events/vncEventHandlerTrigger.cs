using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace openvncViewer.Events
{
    internal static class vncEventHandlerTrigger
    {
        internal static void TriggerEvent(EventHandler handler, Object source)
        {
            if (handler != null)
            {
                handler(source, EventArgs.Empty);
            }
        }

        internal static void TriggerEvent<T>(EventHandler<T> handler, Object source, T args) where T : EventArgs
        {
            if (handler != null)
            {
                handler(source, args);
            }
        }
    }
}
