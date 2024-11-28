using Microsoft.Extensions.DependencyInjection;
using System.Reflection;
using Tomdan.ToolKit.Plugin.Base;

namespace TestPluginA
{
    public class PluginA : IPlugin
    {
        IEventAggregator eventAggregator { get; set; }
        IServiceProvider ServiceProvider { get; set; }
        public string PluginName => "PluginA";

        public string Version => "1.0.0.0";

        public string Description => "A Plugin";

        public string ConfigPath => throw new NotImplementedException();

        public PluginConfig Configs { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }

        public int TimesExecute()
        {
            Console.WriteLine("asdf");
            eventAggregator.Publish("Plugin_A_Test", "this is PluginA ");
            return 0;
        }

        public bool Start()
        {
            return true;
        }

        public bool Stop()
        {
            return false;
        }

        public bool UnLoad()
        {
            return true;
        }
        public void Plugin_B_Test(string str)
        {
        }


        public bool Initialize(IEventAggregator eventAggregator, IServiceProvider serviceProvider, object baseData)
        {
            this.eventAggregator = eventAggregator;
            eventAggregator.Subscribe("Plugin_B_Test", (Object) => Plugin_B_Test((string)Object));
            ServiceProvider = serviceProvider;

            return true;
        }

        public bool BeforeCore()
        {
            return false;
        }

        public bool AfterCore()
        {
            return false
                ;
        }

        public void ConfigureServices(IServiceCollection services, Assembly assembly)
        {
        }


    }
}
