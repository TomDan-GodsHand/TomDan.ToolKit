#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include "IPlugin.h"
#include "IMessageBus.h"

// 插件管理类
class PluginManager {
private:
    struct PluginInfo {
        std::unique_ptr<IPlugin> plugin;
        void* handle; // 动态库句柄
        std::chrono::steady_clock::time_point lastHeartbeat; // 最后心跳时间
        bool active; // 插件是否活跃
        std::string filepath; // 插件文件路径（如果是动态库加载的）
    };
    
    std::vector<PluginInfo> plugins;
    
    // 添加插件的辅助函数
    void addPlugin(std::unique_ptr<IPlugin> plugin, void* handle = nullptr, const std::string& path = "");
    
    // 在指定位置加载插件的辅助函数
    bool loadPluginAtIndex(size_t index, const std::string& filepath);
    
    IMessageBus* messageBus; // 消息总线指针
    const std::chrono::seconds HEARTBEAT_TIMEOUT = std::chrono::seconds(30); // 心跳超时时间

public:
    PluginManager(IMessageBus* bus);
    void registerPlugin(std::unique_ptr<IPlugin> plugin);
    void loadPluginsFromDirectory(const std::string& directory);
    bool loadPlugin(const std::string& filepath); // 加载单个插件
    bool unloadPlugin(size_t index); // 卸载指定插件
    void reloadPlugin(size_t index); // 重新加载指定插件
    void initializeAll();
    void executeAll();
    size_t getPluginCount() const;
    void checkHeartbeats(); // 检查所有插件的心跳
};

#endif // PLUGINMANAGER_H