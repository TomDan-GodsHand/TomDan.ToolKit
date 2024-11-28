using System.Reflection;
using Microsoft.Extensions.DependencyInjection;
using Tomdan.ToolKit.Plugin.Base;
using System.Text.Json;
using System.Text.Json.Serialization;
using Microsoft.Extensions.Logging;

namespace TomDan.ToolKit.Plugin.DDNS;

public class DDNSPlugin : IPlugin
{
    string IPlugin.PluginName => "DDNS";
    public string configPath = "Plugins\\DDNS\\config.json";

    public DDNSPlugin(ILogger<DDNSPlugin> logger)
    {
        this.logger = logger;
    }

    public DDNSPlugin() { }
    string IPlugin.Version => "1.0.0.0";

    string IPlugin.Description => "A Simple DDNS Tool By TomDan Running in TomDan.ToolKit Plugin System";
    private IpMonitor? IpMonitor { get; set; }
    public ILogger<DDNSPlugin> logger { get; }

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
            var config = GetConfig(configPath);
            var ddns = new DDNS(config.secret_id, config.secret_key, config.domain, config.subdomain, serviceProvider.GetService<ILogger<DDNS>>());
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
    /// <summary>
    /// 查询配置文件
    /// </summary>
    /// <param name="path">配置文件路径</param>
    /// <returns>返回 Config 对象，如果读取或解析失败，则返回 null</returns>
    private Config GetConfig(string path)
    {
        try
        {
            // 读取文件内容
            string configString = File.ReadAllText(path);

            // 解析 JSON 配置字符串为 Config 对象
            Config? config = JsonSerializer.Deserialize<Config>(configString, ConfigGenerationContext.Default.Config);
            return config;
        }
        catch (Exception jsonEx)
        {
            logger.LogError(jsonEx.Message);
            return null;
        }
    }
}
[JsonSourceGenerationOptions(WriteIndented = true)]
[JsonSerializable(typeof(Config))]
internal partial class ConfigGenerationContext : JsonSerializerContext
{
}
public class Config
{
    public string? secret_id { get; set; }
    public string? secret_key { get; set; }
    public string? domain { get; set; }
    public string? subdomain { get; set; }

}
