#include "HttpServer.h"
#include "ExampleController.h"
#include <iostream>
#include <thread>
#include <httplib.h>
#include <functional>
#include <string>

// 防止宏冲突
#ifdef DELETE
#undef DELETE
#endif

using namespace std;

HttpServer::HttpServer(int port)
    : running(false), port(port), server(new httplib::Server()) {}

HttpServer::~HttpServer() {
    stop();
    delete server;
}

static const char* toContentType() {
    return "text/plain; charset=utf-8";
}

void HttpServer::addRoute(HttpMethod method, const std::string& path, RouteHandler handler) {
    std::lock_guard<std::mutex> lock(methodRoutesMutex);
    methodRoutes[{method, path}] = handler;
    
    // 为 httplib 注册路由
    if (server) {
        // 创建包装函数，将 httplib 的请求转换为我们的格式
        auto wrapper = [this, path, handler](const httplib::Request& req, httplib::Response& res) {
            // 对于带参数的路径，我们需要处理实际路径
            std::string actualPath = req.path;
            std::string response = handler(actualPath, req.body);
            res.set_content(response, "text/plain");
        };
        
        switch (method) {
            case HttpMethod::GET:
                server->Get(path, wrapper);
                break;
            case HttpMethod::POST:
                server->Post(path, wrapper);
                break;
            case HttpMethod::PUT:
                server->Put(path, wrapper);
                break;
            case HttpMethod::DELETE:
                server->Delete(path, wrapper);
                break;
            case HttpMethod::PATCH:
                server->Patch(path, wrapper);
                break;
        }
    }
}

void HttpServer::addRoute(const std::string& path, RouteHandler handler) {
    addRoute(HttpMethod::GET, path, handler);
}

void HttpServer::start() {
    if (running) return;

    // 自动添加示例接口
    
    running = true;
    serverThread = std::thread([this]() {
        std::cout << "[HttpServer] Starting httplib server on port " << port << std::endl;

        if (!server->listen("0.0.0.0", port)) {
            std::cerr << "[HttpServer] Failed to start server on port " << port << std::endl;
            running = false;
            return;
        }

        std::cout << "[HttpServer] Server stopped" << std::endl;
    });

}

void HttpServer::stop() {
    if (!running) return;
    running = false;

    if (server) {
        server->stop();
    }

    if (serverThread.joinable()) {
        serverThread.join();
    }
    std::cout << "[HttpServer] Server stopped gracefully" << std::endl;
}



std::string HttpServer::handleRequest(const std::string& path, const std::string& body) {
    // 这个方法现在主要用于向后兼容，实际请求由 httplib 直接处理
    // 注意：这个方法只能处理 GET 请求，因为 httplib 回调中我们不知道具体的 HTTP 方法
    // 实际的路由处理已经在 addRoute 中直接注册到 httplib 了
    std::lock_guard<std::mutex> lock(methodRoutesMutex);

    // 尝试查找 GET 方法的路由
    auto it = methodRoutes.find({HttpMethod::GET, path});
    if (it != methodRoutes.end()) {
        return it->second(path, body);
    }
    return "404 Not Found";
}

int HttpServer::getPort() const {
    return port;
}

void HttpServer::registerController(IController* controller) {
    if (controller) {
        std::cout << "[HttpServer] Registering controller: " << controller->getName() << std::endl;
        controller->registerRoutes(this);
    }
}
