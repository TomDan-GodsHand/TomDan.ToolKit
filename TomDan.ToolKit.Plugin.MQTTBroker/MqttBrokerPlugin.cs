using System;
using System.Reflection;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using MQTTnet.Formatter;
using MQTTnet.Formatter.V3;
using Tomdan.ToolKit.Plugin.Base;

namespace TomDan.ToolKit.Plugin.MQTTBroker;


public class MqttBrokerPlugin : IPlugin
{
    public ILogger<MqttBrokerPlugin> logger { get; }
    public string PluginName => "Mqtt Broker";

    public string Version => "1.0.0.0";

    public MqttBroker mqttBroker { get; set; }
    public string Description => "A Simple Mqtt broker by TomDan use Mqttnet ";

    public string ConfigPath => AppDomain.CurrentDomain.BaseDirectory + "Plugins\\MQTTBroker\\config.json";

    public PluginConfig Configs { get; set; }
    public MqttBrokerPlugin(ILogger<MqttBrokerPlugin> logger)
    {
        this.logger = logger;
    }
    public MqttBrokerPlugin() { }

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
 Configs = new();
        Configs.InitConfig(ConfigPath);
            mqttBroker = new(serviceProvider.GetService<ILogger<MqttBroker>>());
            mqttBroker.ConfigMqttServer(Configs.Data["LocalMqttServerIP"], int.Parse(Configs.Data["LocalMqttServerPort"]));
            mqttBroker.AddUser("TomDan", "");
            return true;
        }
        catch (Exception ex)
        {
            throw new PluginException(ex.Message);
        }
    }

    public bool Start()
    {
        if (mqttBroker == null)
            return false;
        mqttBroker.StartMqtt();

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
