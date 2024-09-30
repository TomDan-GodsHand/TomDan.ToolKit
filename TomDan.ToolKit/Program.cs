using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Tomdan.ToolKit.Plugin.Base;
using TomDan.ToolKit.Core.Core;
using TomDan.ToolKit.PluginManagement;

namespace TomDan.ToolKit.Core
{
    internal class Program
    {
        public static async Task Main(string[] args)
        {
            try
            {

                HostApplicationBuilder builder = Host.CreateApplicationBuilder(args);
                // 插件管理器注入
                builder.Services.AddSingleton<AutoResetEvent>(new AutoResetEvent(false));
                builder.Services.AddSingleton<PluginManager>();
                builder.Services.AddSingleton<PluginContext>();
                builder.Services.AddLogging();
                builder.Services.AddSingleton<IEventAggregator, EventAggregator>();
                var pluginManager = new PluginManager();
                pluginManager.LoadPlugin(builder.Services, new Logger<PluginManager>(new LoggerFactory()));

                #region 添加托管的服务
                //核心服务托管
                builder.Services.AddHostedService<Service>();
                //插件服务托管
                builder.Services.AddHostedService<PluginService>();
                #endregion
                using IHost host = builder.Build();

                await host.RunAsync();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }
    }
}
