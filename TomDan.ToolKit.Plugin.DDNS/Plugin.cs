using System;
using System.Reflection;
using Microsoft.Extensions.DependencyInjection;
using Tomdan.ToolKit.Plugin.Base;
using TomDan.ToolKit.Plugin;

namespace TomDan.ToolKit.Plugin.DDNS;

public class Plugin : IPlugin
{
    string IPlugin.PluginName => "DDNS";

    string IPlugin.Version => "1.0.0.0";

    string IPlugin.Description => "A Simple DDNS Tool By TomDan Running in TomDan.ToolKit Plugin System";

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

    bool IPlugin.Initialize(IEventAggregator eventAggregator, IServiceProvider serviceProvider)
    {
        return true;
    }

    bool IPlugin.Start()
    {
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
