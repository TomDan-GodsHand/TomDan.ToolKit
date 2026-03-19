#include "PluginManager.h"
#include <cstring>
#include <filesystem>
#include <iostream>

// 平台特定的动态库加载头文件
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;

PluginManager::PluginManager(IMessageBus *bus) : messageBus(bus) {}

void PluginManager::addPlugin(std::unique_ptr<IPlugin> plugin, void* handle, const std::string& path) {
    if (!plugin) {
        std::cerr << "无法添加空插件" << std::endl;
        return;
    }
    std::string pluginName = plugin->getName();
    PluginInfo info;
    info.plugin = std::move(plugin);
    info.handle = handle;
    info.lastHeartbeat = std::chrono::steady_clock::now();
    info.active = true;
    info.filepath = path; // 存储文件路径
    plugins.push_back(std::move(info));
    std::cout << "已添加插件: " << pluginName << std::endl;
}

bool PluginManager::loadPluginAtIndex(size_t index, const std::string& filepath) {
    try {
        // 检查文件是否为动态库文件
        auto extension = fs::path(filepath).extension().string();
        bool isPluginFile = false;
#ifdef _WIN32
        isPluginFile = (extension == ".dll");
#elif __APPLE__
        isPluginFile = (extension == ".dylib");
#else
        isPluginFile = (extension == ".so");
#endif

        if (!isPluginFile) {
            std::cerr << "不是有效的插件文件: " << filepath << std::endl;
            return false;
        }

        std::cout << "正在在索引 " << index << " 加载插件: " << filepath << std::endl;

        // 平台特定的动态库加载
#ifdef _WIN32
        // Windows 加载
        HMODULE handle = LoadLibraryA(filepath.c_str());
        if (!handle) {
            DWORD error = GetLastError();
            std::cerr << "加载插件失败 (Windows错误 " << error << ")" << std::endl;
            return false;
        }

        // 获取创建插件的函数
        CreatePluginFunc createPlugin =
            (CreatePluginFunc)GetProcAddress(handle, "createPlugin");
        if (!createPlugin) {
            std::cerr << "找不到createPlugin函数 (Windows错误 "
                      << GetLastError() << ")" << std::endl;
            FreeLibrary(handle);
            return false;
        }
#else
        // Linux/Unix/macOS 加载
        void *handle = dlopen(filepath.c_str(), RTLD_LAZY);
        if (!handle) {
            std::cerr << "加载插件失败: " << dlerror() << std::endl;
            return false;
        }

        // 获取创建插件的函数
        CreatePluginFunc createPlugin =
            (CreatePluginFunc)dlsym(handle, "createPlugin");
        if (!createPlugin) {
            std::cerr << "找不到createPlugin函数: " << dlerror() << std::endl;
            dlclose(handle);
            return false;
        }
#endif

        // 创建插件实例
        IPlugin *plugin = createPlugin(messageBus);
        if (!plugin) {
            std::cerr << "创建插件实例失败" << std::endl;
#ifdef _WIN32
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
            return false;
        }

        // 准备新的插件信息
        PluginInfo info;
        info.plugin.reset(plugin);
        info.handle = handle;
        info.lastHeartbeat = std::chrono::steady_clock::now();
        info.active = true;
        info.filepath = filepath;

        // 如果索引超出范围，添加到末尾
        if (index >= plugins.size()) {
            plugins.push_back(std::move(info));
        } else {
            // 替换指定位置的插件
            plugins[index] = std::move(info);
        }

        std::cout << "成功在索引 " << index << " 加载插件: " << plugin->getName() << std::endl;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "在索引 " << index << " 加载插件时发生异常: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "在索引 " << index << " 加载插件时发生未知异常" << std::endl;
        return false;
    }
}

void PluginManager::registerPlugin(std::unique_ptr<IPlugin> plugin) {
  try {
    addPlugin(std::move(plugin), nullptr);
  } catch (const std::exception &e) {
    std::cerr << "注册插件时发生异常: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "注册插件时发生未知异常" << std::endl;
  }
}

bool PluginManager::loadPlugin(const std::string &filepath) {
  try {
    auto extension = fs::path(filepath).extension().string();
    auto filename = fs::path(filepath).filename().string();
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
      std::cout << "正在加载插件: " << filepath << std::endl;

      // 平台特定的动态库加载
#ifdef _WIN32
      // Windows 加载
      HMODULE handle = LoadLibraryA(filepath.c_str());
      if (!handle) {
        DWORD error = GetLastError();
        std::cerr << "加载插件失败 (Windows错误 " << error << ")"
                  << std::endl;
        return false;
      }

      // 获取创建插件的函数
      CreatePluginFunc createPlugin =
          (CreatePluginFunc)GetProcAddress(handle, "createPlugin");
      if (!createPlugin) {
        std::cerr << "找不到createPlugin函数 (Windows错误 "
                  << GetLastError() << ")" << std::endl;
        FreeLibrary(handle);
        return false;
      }
#else
      // Linux/Unix/macOS 加载
      void* handle = dlopen(filepath.c_str(), RTLD_LAZY);
      if (!handle) {
        const char* error = dlerror();
        std::cerr << "加载插件失败: " << (error ? error : "未知错误") << std::endl;
        throw std::runtime_error(std::string("加载插件失败: ") + (error ? error : "未知错误"));
      }

      // 获取创建插件的函数
      CreatePluginFunc createPlugin =
          (CreatePluginFunc)dlsym(handle, "createPlugin");
      if (!createPlugin) {
        std::cerr << "找不到createPlugin函数: " << dlerror()
                  << std::endl;
        dlclose(handle);
        throw std::runtime_error("找不到createPlugin函数");
      }
#endif

      // 创建插件实例
      IPlugin *plugin = createPlugin(messageBus);
      if (!plugin) {
        std::cerr << "创建插件实例失败" << std::endl;
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        throw std::runtime_error("创建插件实例失败");
      }
      // 添加到插件列表
      addPlugin(std::unique_ptr<IPlugin>(plugin), handle, filepath);
    }
  } catch (const std::exception &e) {
    std::cerr << "加载插件失败: " << filepath << " - " << e.what()
              << std::endl;
    return false;
  }
  return true;
}

bool PluginManager::unloadPlugin(size_t index) {
  try {
    if (index >= plugins.size()) {
      std::cerr << "无效的插件索引" << std::endl;
      return false;
    }

    auto &pluginInfo = plugins[index];
    std::string pluginName = pluginInfo.plugin->getName();
    std::cout << "正在卸载插件: " << pluginName << std::endl;

    // 尝试使用插件导出的destroyPlugin接口销毁插件实例
    if (pluginInfo.handle) {
      // 获取destroyPlugin函数
      typedef void (*DestroyPluginFunc)(IPlugin*);
#ifdef _WIN32
      DestroyPluginFunc destroyPlugin = (DestroyPluginFunc)GetProcAddress(
          static_cast<HMODULE>(pluginInfo.handle), "destroyPlugin");
#else
      DestroyPluginFunc destroyPlugin = (DestroyPluginFunc)dlsym(
          pluginInfo.handle, "destroyPlugin");
#endif
      
      if (destroyPlugin) {
        // 使用插件导出的接口销毁插件
        destroyPlugin(pluginInfo.plugin.release());
      } else {
        // 回退到默认方式
        pluginInfo.plugin.reset();
      }
    } else {
      // 没有动态库句柄，使用默认方式
      pluginInfo.plugin.reset();
    }

    // 卸载动态库
#ifdef _WIN32
    if (pluginInfo.handle) {
      if (!FreeLibrary(static_cast<HMODULE>(pluginInfo.handle))) {
        DWORD error = GetLastError();
        std::cerr << "卸载插件库失败 (Windows错误 " << error << ")" << std::endl;
        return false;
      }
    }
#else
    if (pluginInfo.handle) {
      if (dlclose(pluginInfo.handle) != 0) {
        std::cerr << "卸载插件库失败: " << dlerror() << std::endl;
        return false;
      }
    }
#endif

    pluginInfo.handle = nullptr;
    pluginInfo.active = false;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "卸载插件时发生异常: " << e.what() << std::endl;
    return false;
  } catch (...) {
    std::cerr << "卸载插件时发生未知异常" << std::endl;
    return false;
  }
}

void PluginManager::loadPluginsFromDirectory(const std::string &directory) {
  try {
    // 使用标准库遍历目录
    for (const auto &entry : fs::directory_iterator(directory)) {
      try {
        if (entry.is_regular_file()) {
          loadPlugin(entry.path().string());
        }
      } catch (const std::exception &e) {
        std::cerr << "从文件加载插件失败 " << entry.path().string()
                  << ": " << e.what() << std::endl;
        // 继续加载其他插件
      } catch (...) {
        std::cerr << "从文件加载插件时发生未知异常 " 
                  << entry.path().string() << std::endl;
        // 继续加载其他插件
      }
    }
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "无法打开插件目录: " << directory << " - "
              << e.what() << std::endl;
    return;
  } catch (const std::exception &e) {
    std::cerr << "加载插件目录函数中发生异常: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "加载插件目录函数中发生未知异常" << std::endl;
  }
}

void PluginManager::initializeAll() {
  for (auto &pluginInfo : plugins) {
    try {
      if (!pluginInfo.plugin) {
        std::cerr << "插件实例为空，跳过初始化" << std::endl;
        continue;
      }
      pluginInfo.plugin->initialize();
      pluginInfo.lastHeartbeat = std::chrono::steady_clock::now();
    } catch (const std::exception &e) {
      std::cerr << "初始化插件 " << pluginInfo.plugin->getName() 
                << " 时发生异常: " << e.what() << std::endl;
      pluginInfo.active = false;
    } catch (...) {
      std::cerr << "初始化插件 " << pluginInfo.plugin->getName() << " 时发生未知异常" << std::endl;
      pluginInfo.active = false;
    }
  }
}

void PluginManager::executeAll() {
  for (auto &pluginInfo : plugins) {
    try {
      if (!pluginInfo.plugin) {
        std::cerr << "插件实例为空，跳过执行" << std::endl;
        continue;
      }
      if (!pluginInfo.active) {
        std::cout << "插件 " << pluginInfo.plugin->getName() << " 处于非活动状态，跳过执行" << std::endl;
        continue;
      }
      pluginInfo.plugin->execute();
      pluginInfo.lastHeartbeat = std::chrono::steady_clock::now();
    } catch (const std::exception &e) {
      std::cerr << "执行插件 " << pluginInfo.plugin->getName() 
                << " 时发生异常: " << e.what() << std::endl;
      pluginInfo.active = false;
    } catch (...) {
      std::cerr << "执行插件 " << pluginInfo.plugin->getName() << " 时发生未知异常" << std::endl;
      pluginInfo.active = false;
    }
  }
}

size_t PluginManager::getPluginCount() const { return plugins.size(); }

void PluginManager::checkHeartbeats() {
  try {
    auto now = std::chrono::steady_clock::now();
    for (size_t i = 0; i < plugins.size(); ++i) {
      auto &pluginInfo = plugins[i];
      if (!pluginInfo.plugin) {
        std::cerr << "索引 " << i << " 处的插件实例为空，跳过心跳检查" << std::endl;
        pluginInfo.active = false;
        continue;
      }
      
      if (pluginInfo.active) {
        try {
          // 检查插件是否响应心跳
          if (!pluginInfo.plugin->heartbeat()) {
            std::cerr << "插件 " << pluginInfo.plugin->getName()
                      << " 心跳检查失败" << std::endl;
            pluginInfo.active = false;
            reloadPlugin(i);
          } else {
            // 更新心跳时间
            pluginInfo.lastHeartbeat = now;
          }
        } catch (const std::exception &e) {
          std::cerr << "插件 " << pluginInfo.plugin->getName()
                    << " 心跳检查时发生异常: " << e.what() << std::endl;
          pluginInfo.active = false;
          reloadPlugin(i);
        } catch (...) {
          std::cerr << "插件 " << pluginInfo.plugin->getName() << " 心跳检查时发生未知异常" << std::endl;
          pluginInfo.active = false;
          reloadPlugin(i);
        }
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "心跳检查函数中发生异常: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "心跳检查函数中发生未知异常" << std::endl;
  }
}

void PluginManager::reloadPlugin(size_t index) {
  try {
    if (index >= plugins.size()) {
      std::cerr << "无效的插件索引" << std::endl;
      return;
    }

    auto &pluginInfo = plugins[index];
    std::string pluginName = pluginInfo.plugin ? pluginInfo.plugin->getName() : "未知插件";
    std::string filepath = pluginInfo.filepath;
    
    std::cout << "正在重新加载插件: " << pluginName
              << "，路径: " << (filepath.empty() ? "无（内存插件）" : filepath) << std::endl;

    // 如果是通过registerPlugin注册的内存插件，没有文件路径，只能重新激活
    if (filepath.empty()) {
      std::cout << "插件 " << pluginName << " 是内存插件，只能重新激活" << std::endl;
      pluginInfo.active = true;
      pluginInfo.lastHeartbeat = std::chrono::steady_clock::now();
      return;
    }

    // 保存文件路径，因为unloadPlugin不会清除它
    std::string savedFilepath = filepath;
    
    // 1. 卸载当前插件
    bool unloadSuccess = unloadPlugin(index);
    if (!unloadSuccess) {
      std::cerr << "卸载插件失败，但仍尝试重新加载" << std::endl;
    }
    
    // 2. 重新加载插件到相同位置
    bool loadSuccess = loadPluginAtIndex(index, savedFilepath);
    if (!loadSuccess) {
      std::cerr << "重新加载插件失败: " << savedFilepath << std::endl;
      // 插件状态已经在loadPluginAtIndex中处理
    } else {
      std::cout << "成功重新加载插件: " << pluginName << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "重新加载插件时发生异常: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "重新加载插件时发生未知异常" << std::endl;
  }
}