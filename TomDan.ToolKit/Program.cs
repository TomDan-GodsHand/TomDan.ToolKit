using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using TomDan.ToolKit.Core.Core;
using TomDan.ToolKit.PluginManagement;

namespace TomDan.ToolKit.Core
{
    internal class Program
    {
        public static async Task Main(string[] args)
        {

            HostApplicationBuilder builder = Host.CreateApplicationBuilder(args);

            #region 添加托管的服务
            //核心服务托管
            builder.Services.AddHostedService<Service>();
            //插件服务托管
            builder.Services.AddHostedService<PluginService>();
            #endregion
        }
    }
}
