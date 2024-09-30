﻿using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;

namespace TomDan.ToolKit.PluginManagement
{
    public class PluginService(PluginManager pluginManager, ILogger<PluginService> logger, AutoResetEvent autoResetEvent) : IHostedService
    {
        private Task executingTask;
        private CancellationTokenSource cts = new CancellationTokenSource();
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
        private async Task ExecuteAsync()
        {
            await Task.Delay(3000);

            pluginManager.Init();
            pluginManager.BeforeCore();
            autoResetEvent.Set();
            autoResetEvent.WaitOne();
            pluginManager.AfterCore();
            pluginManager.Start();
            while (!cts.Token.IsCancellationRequested)
            {
                // 主循环逻辑
                try
                {
                    pluginManager.TimesExecute();
                }
                catch (Exception ex) { logger.LogError(ex, ex.Message); }
                Thread.Sleep(1000);
            }
            logger.LogInformation($"Pluigns Start End");
            pluginManager.Stop();
        }

    }
}