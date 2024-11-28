using Microsoft.Extensions.DependencyInjection;
using System.Reflection;

namespace Tomdan.ToolKit.Plugin.Base
{
    public interface IPlugin
    {
        string PluginName { get; }
        string Version { get; }
        string Description { get; }
        string ConfigPath { get; }
        PluginConfig Configs { get; set; }
        int TimesExecute();

        /// <summary>
        ///  // 插件自己的服务注册逻辑
        ///  例如
        ///  services.AddScoped<MyPluginService>()
        /// </summary>
        /// <param name="services"></param>
        void ConfigureServices(IServiceCollection services, Assembly assembly);
        /// <summary>
        /// 插件初始化方法，生命周期第一步 ，在这步执行你自己的资源加载，注册事件，订阅事件等，对于事件的依赖关系请在插件说明文档中写明，包括版本号
        /// </summary>
        /// <param name="eventAggregator">
        /// 事件聚合器，使用它注册和订阅事件
        /// </param>
        /// <returns>
        /// 返回是否初始化成功
        /// </returns>
        bool Initialize(IEventAggregator eventAggregator, IServiceProvider serviceProvider = null, object baseData = null);
        bool BeforeCore();
        bool AfterCore();

        /// <summary>
        /// 在这里开启插件的循环，保持插件的运行.这是插件的核心阶段，通常还包括响应事件或执行命令
        /// </summary>
        /// <returns>
        /// 返回是否开启成功
        /// </returns>
        bool Start();

        /// <summary>
        /// 停止插件的运行，释放资源，保存状态
        /// </summary>
        /// <returns></returns>
        bool Stop();

        /// <summary>
        /// 插件被从内存中卸载并销毁。此阶段通常由插件管理器执行，确保插件不再被引用。
        /// </summary>
        /// <returns></returns>
        bool UnLoad();
    }
}
