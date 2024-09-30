using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tomdan.ToolKit.Plugin.Base;
using TomDan.ToolKit.PluginManagement;

namespace TomDan.ToolKit.Core.Core
{
    public class Service(IServiceProvider serviceProvider, ILogger<Service> logger, PluginContext context, AutoResetEvent autoResetEvent) : IHostedService
    {
        private Task executingTask;
        private CancellationTokenSource cts = new();
        private async Task ExecuteAsync()
        {
            await StartSchedule();

        }
        public Task StopAsync(CancellationToken cancellationToken)
        {
            cts.Cancel();
            return executingTask ?? Task.CompletedTask;
        }

        public Task StartAsync(CancellationToken cancellationToken)
        {
            executingTask = Task.Run(ExecuteAsync, cancellationToken);
            return Task.CompletedTask;
        }

        private async Task StartSchedule()
        {
            try
            {

                autoResetEvent.WaitOne();// 等待插件系统完成开始前的步骤
                logger.LogInformation("Core start");


                logger.LogInformation("Core Setup Success");
                autoResetEvent.Set();// 告诉插件系统完成加载


            }
            catch (Exception ex)
            {
                logger.LogError(ex.StackTrace + "\r\n" + ex.Message);
            }
        }

    }
}
