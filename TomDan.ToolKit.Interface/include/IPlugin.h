#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <string>

class IPlugin;
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
typedef IPlugin* (*CreatePluginFunc)(void* context);

// 插件接口类
class PLUGIN_API IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual std::string getName() const = 0;
    virtual void initialize() = 0;
    virtual void execute() = 0;
    virtual std::string getPath() const =0; // 获取插件路径
    virtual std::string setPath(const std::string &path); // 设置插件路径
    virtual bool heartbeat() = 0; // 心跳方法，返回插件是否正常运行
};

// 插件导出函数声明
extern "C" PLUGIN_API IPlugin* createPlugin(void* context);

extern "C" PLUGIN_API void destroyPlugin(IPlugin* plugin);

// 插件路径相关导出函数声明
extern "C" PLUGIN_API const char* getPluginPath(IPlugin* plugin);
extern "C" PLUGIN_API void setPluginPath(IPlugin* plugin, const char* path);

#endif // IPLUGIN_H