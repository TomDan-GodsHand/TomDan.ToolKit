#include "PluginManager.h"
#include <iostream>
#include <cstring>
#include <filesystem>

// 平台特定的动态库加载头文件
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

namespace fs = std::filesystem;

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
    try {
        // 使用标准库遍历目录
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                std::string extension = entry.path().extension().string();
                
                // 检查是否为动态库文件（根据平台）
                bool isPluginFile = false;
#ifdef _WIN32
                isPluginFile = (extension == ".dll");
#elif __APPLE__
                isPluginFile = (extension == ".dylib");
#else
                isPluginFile = (extension == ".so");
#endif
                
                if (isPluginFile) {
                    std::string filepath = entry.path().string();
                    std::cout << "Loading plugin: " << filepath << std::endl;

                    // 平台特定的动态库加载
#ifdef _WIN32
                    // Windows 加载
                    HMODULE handle = LoadLibraryA(filepath.c_str());
                    if (!handle) {
                        DWORD error = GetLastError();
                        std::cerr << "Failed to load plugin (Windows error " << error << ")" << std::endl;
                        continue;
                    }
                    
                    // 获取创建插件的函数
                    CreatePluginFunc createPlugin = (CreatePluginFunc)GetProcAddress(handle, "createPlugin");
                    if (!createPlugin) {
                        std::cerr << "Failed to find createPlugin function (Windows error " << GetLastError() << ")" << std::endl;
                        FreeLibrary(handle);
                        continue;
                    }
#else
                    // Linux/Unix/macOS 加载
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
#endif

                    // 创建插件实例
                    IPlugin* plugin = createPlugin(messageBus);
                    if (!plugin) {
                        std::cerr << "Failed to create plugin instance" << std::endl;
#ifdef _WIN32
                        FreeLibrary(handle);
#else
                        dlclose(handle);
#endif
                        continue;
                    }

                    // 添加到插件列表
                    PluginInfo info;
                    info.plugin.reset(plugin);
                    info.handle = reinterpret_cast<void*>(handle);
                    info.lastHeartbeat = std::chrono::steady_clock::now();
                    info.active = true;
                    plugins.push_back(std::move(info));

                    std::cout << "Successfully loaded plugin: " << plugin->getName() << std::endl;
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to open plugin directory: " << directory << " - " << e.what() << std::endl;
        return;
    }
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