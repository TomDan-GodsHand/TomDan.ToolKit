#include "PluginManager.h"
#include <iostream>
#include <dlfcn.h>
#include <dirent.h>
#include <cstring>

PluginManager::PluginManager(IMessageBus* bus) : messageBus(bus) {}

void PluginManager::registerPlugin(std::unique_ptr<IPlugin> plugin) {
    PluginInfo info;
    info.plugin = std::move(plugin);
    info.handle = nullptr;
    info.lastHeartbeat = std::chrono::steady_clock::now();
    info.active = true;
    plugins.push_back(std::move(info));
}

void PluginManager::loadPluginsFromDirectory(const std::string& directory) {
    DIR* dir = opendir(directory.c_str());
    if (!dir) {
        std::cerr << "Failed to open plugin directory: " << directory << std::endl;
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) {
            std::string filename = entry->d_name;
            // 检查是否为动态库文件
            if (filename.size() > 3 && filename.substr(filename.size() - 3) == ".so") {
                std::string filepath = directory + "/" + filename;
                std::cout << "Loading plugin: " << filepath << std::endl;

                // 加载动态库
                void* handle = dlopen(filepath.c_str(), RTLD_LAZY);
                if (!handle) {
                    std::cerr << "Failed to load plugin: " << dlerror() << std::endl;
                    continue;
                }

                // 获取创建插件的函数
                CreatePluginFunc createPlugin = (CreatePluginFunc)dlsym(handle, "createPlugin");
                if (!createPlugin) {
                    std::cerr << "Failed to find createPlugin function: " << dlerror() << std::endl;
                    dlclose(handle);
                    continue;
                }

                // 创建插件实例
                IPlugin* plugin = createPlugin(messageBus);
                if (!plugin) {
                    std::cerr << "Failed to create plugin instance" << std::endl;
                    dlclose(handle);
                    continue;
                }

                // 添加到插件列表
                PluginInfo info;
                info.plugin.reset(plugin);
                info.handle = handle;
                info.lastHeartbeat = std::chrono::steady_clock::now();
                info.active = true;
                plugins.push_back(std::move(info));

                std::cout << "Successfully loaded plugin: " << plugin->getName() << std::endl;
            }
        }
    }

    closedir(dir);
}

void PluginManager::initializeAll() {
    for (auto& pluginInfo : plugins) {
        pluginInfo.plugin->initialize();
        pluginInfo.lastHeartbeat = std::chrono::steady_clock::now();
    }
}

void PluginManager::executeAll() {
    for (auto& pluginInfo : plugins) {
        pluginInfo.plugin->execute();
        pluginInfo.lastHeartbeat = std::chrono::steady_clock::now();
    }
}

size_t PluginManager::getPluginCount() const {
    return plugins.size();
}

void PluginManager::checkHeartbeats() {
    auto now = std::chrono::steady_clock::now();
    for (size_t i = 0; i < plugins.size(); ++i) {
        auto& pluginInfo = plugins[i];
        if (pluginInfo.active) {
            // 检查插件是否响应心跳
            if (!pluginInfo.plugin->heartbeat()) {
                std::cerr << "Plugin " << pluginInfo.plugin->getName() << " failed heartbeat check" << std::endl;
                pluginInfo.active = false;
                reloadPlugin(i);
            } else {
                // 更新心跳时间
                pluginInfo.lastHeartbeat = now;
            }
        }
    }
}

void PluginManager::reloadPlugin(size_t index) {
    if (index >= plugins.size()) {
        std::cerr << "Invalid plugin index" << std::endl;
        return;
    }

    auto& pluginInfo = plugins[index];
    std::string pluginName = pluginInfo.plugin->getName();
    std::cout << "Reloading plugin: " << pluginName << std::endl;

    // 这里简化处理，实际应该重新加载动态库
    // 由于我们没有存储动态库路径，这里只是重置插件状态
    std::cout << "Plugin " << pluginName << " reloaded (simulated)" << std::endl;
    pluginInfo.active = true;
    pluginInfo.lastHeartbeat = std::chrono::steady_clock::now();
}