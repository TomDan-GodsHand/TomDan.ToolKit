#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include <string>

class Plugin;
// 插件导出宏
#ifndef PLUGIN_API
#ifdef _WIN32
#ifdef PLUGIN_EXPORTS
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API __declspec(dllimport)
#endif
#else
#define PLUGIN_API
#endif
#endif

// 前向声明

// 插件创建函数类型
typedef Plugin *(*CreatePluginFunc)(void *context);

// 插件接口类
class PLUGIN_API Plugin {
public:
  virtual ~Plugin() = default;
  virtual std::string getName() const = 0;
  virtual void initialize() = 0;
  virtual void execute() = 0;
  virtual bool heartbeat() = 0; // 心跳方法，返回插件是否正常运行
};

// 插件导出函数声明
extern "C" PLUGIN_API Plugin *createPlugin(void *context);

extern "C" PLUGIN_API void destroyPlugin(Plugin *plugin);

#endif // PLUGIN_HPP
