using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tomdan.ToolKit.Plugin.Base
{
    public interface IEventAggregator
    {
        void Subscribe(string eventName, Action<object> handler);
        void Publish(string eventName, object data);
    }
}
