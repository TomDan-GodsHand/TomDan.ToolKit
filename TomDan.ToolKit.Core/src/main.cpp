#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "../include/PluginManager.h"
#include "../include/MessageBus.h"

std::atomic<bool> g_running(true);
std::mutex g_mutex;
std::condition_variable g_cv;

// 处理用户输入的线程函数
void inputThread() {
    std::string input;
    while (g_running) {
        std::cout << "\nEnter 'quit' to exit: " << std::endl;
        std::getline(std::cin, input);
        if (input == "quit") {
            g_running = false;
            g_cv.notify_one();
            break;
        }
    }
}

int main() {
    // 创建消息总线
    MessageBus messageBus;

    // 创建插件管理器
    PluginManager pluginManager(&messageBus);

    // 加载插件目录
    std::cout << "Loading plugins from directory..." << std::endl;
    pluginManager.loadPluginsFromDirectory("./plugins");

    // 初始化所有插件
    std::cout << "\nInitializing all plugins..." << std::endl;
    pluginManager.initializeAll();

    // 执行所有插件
    std::cout << "\nExecuting all plugins..." << std::endl;
    pluginManager.executeAll();

    std::cout << "\nTotal plugins: " << pluginManager.getPluginCount() << std::endl;
    std::cout << "\nCore is running. Press 'quit' to exit." << std::endl;

    // 启动用户输入线程
    std::thread input_thread(inputThread);

    // 主循环，定期检查插件心跳
    while (g_running) {
        // 检查插件心跳
        pluginManager.checkHeartbeats();

        // 等待一段时间
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // 等待输入线程结束
    input_thread.join();

    std::cout << "\nCore is shutting down..." << std::endl;

    return 0;
}