#include "IPlugin.h"
#include "IMessageBus.h"
#include <iostream>
#include <string>

class ExamplePlugin : public IPlugin {
private:
    IMessageBus* messageBus;
    int subscriptionId;
    std::string pluginPath; // 插件路径
    int heartbeatCount; // 心跳计数器

public:
    ExamplePlugin(IMessageBus* bus) : messageBus(bus), subscriptionId(0), heartbeatCount(0) {}

    std::string getName() const override {
        return "ExamplePlugin";
    }

    std::string getPath() const override {
        return pluginPath;
    }

    std::string setPath(const std::string &path) override {
        pluginPath = path;
        return pluginPath;
    }

    void initialize() override {
        std::cout << "Initializing " << getName() << std::endl;
        
        // 订阅消息
        subscriptionId = messageBus->subscribe("test", [this](const std::string& message) {
            std::cout << getName() << " received message: " << message << std::endl;
        });
        
        // 发布消息
        messageBus->publish("example", "Example plugin initialized");
    }

    void execute() override {
        std::cout << "Executing " << getName() << std::endl;
        // 发布消息
        messageBus->publish("example", "Example plugin executed");
    }

    bool heartbeat() override { return true; }

 
};

// 导出函数
extern "C" PLUGIN_API IPlugin* createPlugin(void* context) {
    return new ExamplePlugin(static_cast<IMessageBus*>(context));
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