using Microsoft.Extensions.Configuration;
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

                // SetLogger(builder.Services);
                builder.Services.AddSingleton<AutoResetEvent>(new AutoResetEvent(false));
                builder.Services.AddLogging();
                PluginService.InitPluginManager(builder.Services);


                #region 添加托管的服务
                //核心服务托管
                builder.Services.AddHostedService<Service>();
                #endregion
                using IHost host = builder.Build();

                await host.RunAsync();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }
        /* 
                private static void SetLogger(IServiceCollection services)
                {
                    var config = new ConfigurationBuilder()
                        .SetBasePath(basePath: Directory.GetCurrentDirectory())
                        .AddJsonFile("appsettings.json", optional: true, reloadOnChange: true)
                        .Build();
                    services.AddLogging(loggingBuilder =>
                    {
                        loggingBuilder.ClearProviders();
                        loggingBuilder.SetMinimumLevel(Microsoft.Extensions.Logging.LogLevel.Trace);
                        loggingBuilder.AddNLog(config);
                    }).BuildServiceProvider();
                } */
    }
}
