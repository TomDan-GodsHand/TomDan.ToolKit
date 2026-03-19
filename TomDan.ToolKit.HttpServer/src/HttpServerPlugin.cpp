#include "ExampleController.h"
#include "IPlugin.h"
#include "IMessageBus.h"
#include "HttpServer.h"
#include "ControllerManager.h"
#include <iostream>
#include <string>
#include <atomic>

// 简单的 HTTP 服务器框架
// 插件主类
class HttpServerPlugin : public IPlugin {
private:
    IMessageBus* messageBus;
    HttpServer server;
    std::atomic<bool> initialized;
    ControllerManager controllerManager;

    std::string pluginPath; // 插件路径
    void RegisterAllController() {
        controllerManager.addController(std::make_unique<ExampleController>());
        controllerManager.registerAllRoutes(&server);
    }

public:
    HttpServerPlugin(IMessageBus* bus) : messageBus(bus), server(8080), initialized(false) {}
    
    std::string getName() const override {
        return "HttpServerPlugin";
    }

    std::string getPath() const override {
        return pluginPath;
    }
    std::string setPath(const std::string &path) override {
        pluginPath = path;
        return pluginPath;
    }
    
    void initialize() override {
        if (initialized) return;
        
        std::cout << "Initializing " << getName() << " on port " << server.getPort() << std::endl;
        
        // 启动 HTTP 服务器
        server.start();

        RegisterAllController();
        
        // 通过消息总线发布插件启动事件
        messageBus->publish("plugin_events", "HttpServerPlugin initialized");
        
        initialized = true;
    }
    
    void execute() override {
        std::cout << "Executing " << getName() << std::endl;
        // 可以在这里执行周期性任务，或者什么都不做（HTTP 服务器已在后台运行）
        // 发布执行事件
        messageBus->publish("plugin_events", "HttpServerPlugin executed");
    }
    
    bool heartbeat() override {
        // 简单的健康检查：服务器是否在运行
        return initialized;
    }
};

// 导出函数
extern "C" PLUGIN_API IPlugin* createPlugin(void* context) {
    return new HttpServerPlugin(static_cast<IMessageBus*>(context));
}

extern "C" PLUGIN_API void destroyPlugin(IPlugin* plugin) {
    delete plugin;
}

extern "C" PLUGIN_API const char* getPluginPath(IPlugin* plugin) {
    thread_local static std::string path;
    path = plugin->getPath();
    return path.c_str();
}

extern "C" PLUGIN_API void setPluginPath(IPlugin* plugin, const char* path) {
    plugin->setPath(std::string(path));
}