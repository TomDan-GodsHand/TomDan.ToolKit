using System;
using System.Reflection;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Tomdan.ToolKit.Plugin.Base;

namespace TomDan.ToolKit.Plugin.MQTTBroker;


public class MqttBrokerPlugin : IPlugin
{
    public ILogger<MqttBrokerPlugin> logger { get; }
    public MqttBrokerPlugin(ILogger<MqttBrokerPlugin> logger)
    {
        this.logger = logger;
    }
    public string PluginName => "Mqtt Broker";

    public string Version => "1.0.0.0";

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

            return true;
        }
        catch (Exception ex)
        {
            throw new PluginException(ex.Message);
        }
        return false;
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
