using System.Reflection;
using Microsoft.Extensions.DependencyInjection;
using Tomdan.ToolKit.Plugin.Base;
using Microsoft.Extensions.Logging;

namespace TomDan.ToolKit.Plugin.DDNS;

public class DDNSPlugin : IPlugin
{
    string IPlugin.PluginName => "DDNS";

    public string ConfigPath => AppDomain.CurrentDomain.BaseDirectory + "Plugins\\DDNS\\config.json";
    string IPlugin.Version => "1.0.0.0";

    string IPlugin.Description => "A Simple DDNS Tool By TomDan Running in TomDan.ToolKit Plugin System";
    private IpMonitor? IpMonitor { get; set; }
    public ILogger<DDNSPlugin> logger { get; }
    public PluginConfig Configs { get; set; }

    public DDNSPlugin() { }
    public DDNSPlugin(ILogger<DDNSPlugin> logger)
    {
        this.logger = logger;
    }


    bool IPlugin.AfterCore()
    {
        return true;
    }

    bool IPlugin.BeforeCore()
    {
        return true;
    }

    void IPlugin.ConfigureServices(IServiceCollection services, Assembly assembly)
    {

    }

    bool IPlugin.Initialize(IEventAggregator eventAggregator, IServiceProvider serviceProvider, object baseData)
    {
        try
        {
            Configs = new PluginConfig();
            Configs.InitConfig(ConfigPath);
            var ddns = new DDNS(Configs.Data["secretId"], Configs.Data["secretKey"], Configs.Data["domain"], Configs.Data["subDomain"], serviceProvider.GetService<ILogger<DDNS>>());
            var ipMonitor = new IpMonitor(ddns, serviceProvider.GetService<ILogger<IpMonitor>>());
            IpMonitor = ipMonitor;
            return true;
        }
        catch (Exception ex) { logger.LogError(ex.Message, ex); }
        return false;
    }

    bool IPlugin.Start()
    {
        IpMonitor.MainLoop();
        return true;
    }

    bool IPlugin.Stop()
    {
        return false;
    }

    int IPlugin.TimesExecute()
    {
        return 0;
    }

    bool IPlugin.UnLoad()
    {
        return false;
    }

}
