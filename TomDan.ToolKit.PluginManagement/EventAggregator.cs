using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomdan.ToolKit.Plugin.Base;

namespace TomDan.ToolKit.PluginManagement
{
    public class EventAggregator : IEventAggregator
    {
        private readonly Dictionary<string, List<Action<object>>> _dicsubscribers = new();

        public void Publish(string eventName, object data)
        {
            if (_dicsubscribers.ContainsKey(eventName))
            {
                foreach (var handler in _dicsubscribers[eventName])
                {
                    handler(data);
                }
            }
        }

        public void Subscribe(string eventName, Action<object> handler)
        {
            if (!_dicsubscribers.ContainsKey(eventName))
            {
                _dicsubscribers[eventName] = new List<Action<object>>();
            }
            _dicsubscribers[eventName].Add(handler);
        }
    }
}
