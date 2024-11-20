using System;
using System.Reflection;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using MQTTnet.Formatter.V3;
using Tomdan.ToolKit.Plugin.Base;

namespace TomDan.ToolKit.Plugin.MQTTBroker;


public class MqttBrokerPlugin : IPlugin
{
    public ILogger<MqttBrokerPlugin> logger { get; }
    public MqttBrokerPlugin(ILogger<MqttBrokerPlugin> logger)
    {
        this.logger = logger;
    }
    public MqttBrokerPlugin() { }
    public string PluginName => "Mqtt Broker";

    public string Version => "1.0.0.0";

    public MqttBroker mqttBroker { get; set; }
    public string Description => "A Simple Mqtt broker by TomDan use Mqttnet ";

    public bool AfterCore()
    {
        return true;
    }

    public bool BeforeCore()
    {
        return true;
    }

    public void ConfigureServices(IServiceCollection services, Assembly assembly)
    {
    }

    public bool Initialize(IEventAggregator eventAggregator, IServiceProvider serviceProvider = null, object baseData = null)
    {
        try
        {
            mqttBroker = new(serviceProvider.GetService<ILogger<MqttBroker>>());
            mqttBroker.ConfigMqttServer("127.0.0.1", 1883);
            mqttBroker.AddUser("TomDan", "");
            mqttBroker.StartMqtt();
            return true;
        }
        catch (Exception ex)
        {
            throw new PluginException(ex.Message);
        }
    }

    public bool Start()
    {
        return true;
    }

    public bool Stop()
    {
        return true;
    }

    public int TimesExecute()
    {
        return 0;
    }

    public bool UnLoad()
    {
        return true;
    }
}
