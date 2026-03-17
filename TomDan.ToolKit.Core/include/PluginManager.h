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
    };
    
    std::vector<PluginInfo> plugins;
    IMessageBus* messageBus; // 消息总线指针
    const std::chrono::seconds HEARTBEAT_TIMEOUT = std::chrono::seconds(30); // 心跳超时时间

public:
    PluginManager(IMessageBus* bus);
    void registerPlugin(std::unique_ptr<IPlugin> plugin);
    void loadPluginsFromDirectory(const std::string& directory);
    void initializeAll();
    void executeAll();
    size_t getPluginCount() const;
    void checkHeartbeats(); // 检查所有插件的心跳
    void reloadPlugin(size_t index); // 重新加载指定插件
};

#endif // PLUGINMANAGER_H