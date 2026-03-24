#include "LocalMessageBus.hpp"
#include "Network/NetworkCore.hpp"
#include "PluginManager.hpp"
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

std::atomic<bool> g_running(true);
std::mutex g_mutex;
std::condition_variable g_cv;

// 处理用户输入的线程函数
void inputThread() {
    std::string input;
    while (g_running) {
        std::cout << "\n输入 'quit' 以退出：" << std::endl;
        std::getline(std::cin, input);
        if (input == "quit") {
            g_running = false;
            g_cv.notify_one();
            break;
        }
    }
}

int main() {
    try {
        NetworkCore net("localhost", 8888);

        std::cout << "核心正在启动..." << std::endl;
        // 创建消息总线
        LocalMessageBus messageBus;

        // 创建插件管理器
        PluginManager pluginManager(&messageBus);

        // 加载插件目录
        std::cout << "正在从目录加载插件..." << std::endl;
        pluginManager.loadPluginsFromDirectory("./plugins");

        // 初始化所有插件
        std::cout << "\n正在初始化所有插件..." << std::endl;
        pluginManager.initializeAll();

        // 执行所有插件
        std::cout << "\n正在执行所有插件..." << std::endl;
        pluginManager.executeAll();

        std::cout << "\n插件总数：" << pluginManager.getPluginCount() << std::endl;
        std::cout << "\n核心正在运行。输入 'quit' 以退出。" << std::endl;

        // 启动用户输入线程
        std::thread input_thread(inputThread);

        // 主循环，定期检查插件心跳
        while (g_running) {
            // 检查插件心跳
            pluginManager.checkHeartbeats();
            if (net.receive().has_value()) {
                auto message = net.receive().value();
                std::cout << "\n收到网络消息：" << message << std::endl;
                // 将消息发布到消息总线
                messageBus.publish("network_message", message);
            }
            // 等待一段时间
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // 等待输入线程结束
        input_thread.join();

        std::cout << "\n核心正在关闭..." << std::endl;

        return 0;
    } catch (const std::system_error &e) {
        std::cerr << "在 main 中捕获到 std::system_error：" << e.what() << " (代码: " << e.code() << ")" << std::endl;
        return 1;
    } catch (const std::exception &e) {
        std::cerr << "在 main 中捕获到 std::exception：" << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "在 main 中捕获到未知异常" << std::endl;
        return 1;
    }
}