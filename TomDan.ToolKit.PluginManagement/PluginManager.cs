using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Tomdan.ToolKit.Plugin.Base;

namespace TomDan.ToolKit.PluginManagement
{
    public class PluginManager
    {
        private readonly ILogger<PluginManager> logger;
        private readonly IEventAggregator eventAggregator;
        private readonly IServiceProvider serviceProvider;

        public PluginManager()
        {

        }
        public PluginManager(ILogger<PluginManager> logger, IEventAggregator eventAggregator, IServiceProvider serviceProvider)
        {
            this.logger = logger;
            this.eventAggregator = eventAggregator;
            this.serviceProvider = serviceProvider;
        }

        #region 加载相关
        public List<string> FindPlugin()
        {
            List<string> pluginPaths = new List<string>();
            try
            {
                //获取程序的基目录
                string path = AppDomain.CurrentDomain.BaseDirectory;
                //合并路径，指向插件所在目录。
                path = Path.Combine(path, "Plugins");
                var PluginJsons = Directory.GetFiles(path, $"*.deps.json", SearchOption.AllDirectories);
                foreach (var jsonPath in PluginJsons)
                {
                    var strs = jsonPath.Split("\\");
                    var fileName = strs[strs.Length - 1];
                    var fileNameStrs = fileName.Split(".");
                    var pluginNameArrays = fileNameStrs.Take(fileNameStrs.Length - 2).ToArray();
                    var pluginName = string.Empty;
                    pluginName = string.Join(".", pluginNameArrays);
                    foreach (string filename in Directory.GetFiles(path, $"{pluginName}.dll", SearchOption.AllDirectories))
                    {
                        pluginPaths.Add(filename);
                    }
                }

            }
            catch (Exception ex)
            {
                logger.LogError(ex.Message);
            }
            return pluginPaths;
        }

        public void LoadPlugin(IServiceCollection services, ILogger<PluginManager> logger)
        {
            var pluginPaths = FindPlugin();
            try
            {
                AppDomain.CurrentDomain.AssemblyResolve += OnAssemblyResolve;
                IEnumerable<string> plugins = pluginPaths.SelectMany(pluginPath =>
                {
                    Assembly pluginAssembly = LoadPlugin(pluginPath, logger);
                    return CreatePlugins(pluginAssembly, services, logger);
                }).ToList();
                var missPluginsList = pluginPaths.Except(plugins).ToList();
                logger.LogInformation($"Plugin Load Success ({plugins.Count()}) :  " + string.Join("\r\n\t" + plugins));
                logger.LogInformation($"Plugin Load Fail ({missPluginsList.Count()}) :  " + string.Join("\r\n\t" + missPluginsList));

            }
            catch (Exception ex)
            {
                logger.LogError(ex.Message);
            }
        }

        public Assembly LoadPlugin(string path, ILogger<PluginManager> logger)
        {
            logger.LogInformation($"Loading pligins from: {path}");
            PluginLoadContext loadContext = new(path);
            try
            {
                var asm = loadContext.LoadFromAssemblyName(new AssemblyName(Path.GetFileNameWithoutExtension(path)));
                return asm;
            }
            catch (Exception ex)
            {
                logger.LogError(ex.Message);
            }
            return null;
        }

        public IEnumerable<string> CreatePlugins(Assembly assembly, IServiceCollection services, ILogger<PluginManager> logger)
        {
            int count = 0;
            Type[] typeVector;
            try
            {
                if (assembly == null)
                    yield break;
                typeVector = assembly.GetExportedTypes();
            }
            catch (Exception ex)
            {
                logger.LogError(ex.Message);
                yield break;
            }
            /* var tlist = new List<Type>();
            foreach (var type in typeVector)
            {
                var a = type.GetInterface("IPlugin");
                if (a != null)
                {
                    tlist.Add(type);
                }
            } */
            foreach (Type type in typeVector)
            {
                if (type.GetInterface("IPlugin") != null)
                {
                    if (typeof(IPlugin).IsAssignableFrom(type))
                    {
                        IPlugin result = Activator.CreateInstance(type) as IPlugin;
                        result.ConfigureServices(services, assembly);
                        services.AddSingleton(typeof(IPlugin), type);
                        count++;
                        yield return assembly.GetName().Name;
                    }
                }
            }

            if (count == 0)
            {
                string availableTypes = string.Join(",", assembly.GetTypes().Select(t => t.FullName));
                logger.LogWarning(
                    $"Can't find any type which implements IPlugin in {assembly} from {assembly.Location}.\n" +
                    $"Available types: {availableTypes}");
            }
        }

        private static Assembly OnAssemblyResolve(object sender, ResolveEventArgs args)
        {
            // 解析缺失的程序集名称
            string assemblyName = new AssemblyName(args.Name).Name;

            // 搜索公共库或者其他依赖的路径
            string assemblyPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Dependencies", $"{assemblyName}.dll");
            string pluginpath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Plugins");
            var dirs = Directory.GetDirectories(pluginpath);
            foreach (var dir in dirs)
            {
                var path = Path.Combine(dir, "Dependencies", $"{assemblyName}.dll");
                if (File.Exists(path))
                {
                    // 如果找到该程序集，则加载它
                    return Assembly.LoadFrom(assemblyPath);
                }
            }

            if (File.Exists(assemblyPath))
            {
                // 如果找到该程序集，则加载它
                return Assembly.LoadFrom(assemblyPath);
            }

            // 未找到依赖项时，返回 null
            return null;
        }
        #endregion
        #region 生命周期
        public void Init()
        {
            var plugins = serviceProvider.GetServices<IPlugin>();
            foreach (var plugin in plugins)
            {
                try
                {
                    logger.LogInformation($"Plugin Initialize :{plugin.PluginName}");
                    plugin.Initialize(eventAggregator, serviceProvider);
                }
                catch (Exception ex) { logger.LogError($"Plugin {plugin.PluginName}:  {ex}"); }
            }
        }

        internal void BeforeCore()
        {
            var plugins = serviceProvider.GetServices<IPlugin>();
            foreach (var plugin in plugins)
            {
                try
                {
                    logger.LogInformation($"Plugin BeforeCore :{plugin.PluginName}");
                    plugin.BeforeCore();
                }
                catch (Exception ex) { logger.LogError($"Plugin {plugin.PluginName}:  {ex}"); }
            }
        }

        internal void AfterCore()
        {
            var plugins = serviceProvider.GetServices<IPlugin>();
            foreach (var plugin in plugins)
            {
                try
                {
                    logger.LogInformation($"Plugin AfterCore :{plugin.PluginName}");
                    plugin.AfterCore();
                }
                catch (Exception ex) { logger.LogError($"Plugin {plugin.PluginName}:  {ex}"); }
            }
        }

        public void Start()
        {
            var plugins = serviceProvider.GetServices<IPlugin>();
            foreach (var plugin in plugins)
            {
                try
                {
                    logger.LogInformation($"Plugin Start :{plugin.PluginName}");
                    plugin.Start();
                }
                catch (Exception ex) { logger.LogError($"Plugin {plugin.PluginName}:  {ex}"); }
            }
        }

        public void TimesExecute()
        {
            var plugins = serviceProvider.GetServices<IPlugin>();
            foreach (var plugin in plugins)
            {
                logger.LogInformation($"Plugin :{plugin.PluginName},Ex:{plugin.TimesExecute()}");
            }
        }

        public void Stop()
        {
            var plugins = serviceProvider.GetServices<IPlugin>();
            foreach (var plugin in plugins)
            {
                if (plugin.Stop())
                {
                    logger.LogInformation($"Plugin :{plugin.PluginName}, End success");
                }
                else
                {
                    logger.LogInformation($"Plugin :{plugin.PluginName}, End success");
                }
            }
        }
        #endregion
    }

}
