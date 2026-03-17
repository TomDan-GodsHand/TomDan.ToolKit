#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <atomic>
#include <thread>
#include <mutex>
#include <map>
#include <functional>
#include <string>
#include <utility>

namespace httplib { class Server; }

enum class HttpMethod { GET, POST, PUT, DELETE, PATCH };

struct RouteInfo {
    HttpMethod method;
    std::string path;
    std::function<std::string(const std::string&, const std::string&)> handler;
};

// 通过宏模拟 C# 特性 (Attribute) 风格的路由注册。
// 使用方式：
//   HTTP_GET(getRoot, "/")
//   std::string getRoot(const std::string& path, const std::string& body);
// 然后在控制器注册函数中调用 REGISTER_ROUTE(GET, getRoot);

#define HTTP_ROUTE(method, name, path) \
    static inline void __register_##method##_##name(HttpServer* server, auto* instance) { \
        server->addRoute(HttpMethod::method, path, [instance](const std::string& p, const std::string& b) { \
            return instance->name(p, b); \
        }); \
    }

#define HTTP_GET(name, path) HTTP_ROUTE(GET, name, path)
#define HTTP_POST(name, path) HTTP_ROUTE(POST, name, path)
#define HTTP_PUT(name, path) HTTP_ROUTE(PUT, name, path)
#define HTTP_DELETE(name, path) HTTP_ROUTE(DELETE, name, path)
#define HTTP_PATCH(name, path) HTTP_ROUTE(PATCH, name, path)

#define REGISTER_ROUTE(method, name) __register_##method##_##name(server, this)

/**
 * @brief 简单的 HTTP 服务器框架
 *
 * 提供路由注册和请求处理功能，可集成实际的 HTTP 库（如 cpp-httplib）。
 * 当前实现为模拟服务器，仅用于演示框架用法。
 */
class HttpServer {
public:
    // 路由处理函数：传入请求路径和请求体，返回响应体
    using RouteHandler = std::function<std::string(const std::string& path, const std::string& body)>;

private:
    std::atomic<bool> running;
    std::thread serverThread;
    int port;

    // 按方法和路径的路由表
    // 支持 httplib 正则路径（例如 R"(^/api/items/(\d+)$)"）
    std::map<std::pair<HttpMethod, std::string>, RouteHandler> methodRoutes;
    std::mutex methodRoutesMutex;
    
    // 辅助函数：从请求中提取路径参数
    std::string extractPathParam(const std::string& registeredPath, const std::string& actualPath);

    // httplib 服务器实例
    httplib::Server* server;

public:

    /**
     * @brief 构造 HttpServer 实例
     * @param port 监听端口，默认为 8080
     */
    explicit HttpServer(int port = 8080);
    
    /**
     * @brief 析构函数，自动停止服务器
     */
    ~HttpServer();
    
    // 禁用拷贝和赋值
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    
    /**
     * @brief 启动 HTTP 服务器（后台线程）
     * 
     * 如果服务器已在运行，则无操作。
     */
    void start();
    
    /**
     * @brief 停止 HTTP 服务器并等待线程结束
     */
    void stop();
    
    /**
     * @brief 注册路由（指定 HTTP 方法）
     * @param method HTTP 方法
     * @param path HTTP 路径（支持正则，例如 R"(^/api/items/(\d+)$)"）
     * @param handler 处理函数，接受实际请求路径和请求体，返回响应字符串
     */
    void addRoute(HttpMethod method, const std::string& path, RouteHandler handler);

    /**
     * @brief 注册路由（默认使用 GET 方法）
     * @param path HTTP 路径（支持正则，例如 R"(^/api/items/(\d+)$)"）
     * @param handler 处理函数，接受实际请求路径和请求体，返回响应字符串
     */
    void addRoute(const std::string& path, RouteHandler handler);


    /**
     * @brief 处理请求（供实际 HTTP 库回调使用），仅用于兼容旧接口
     * @param path 请求路径
     * @param body 请求体
     * @return 响应字符串，若路由不存在则返回 "404 Not Found"
     */
    std::string handleRequest(const std::string& path, const std::string& body);
    
    /**
     * @brief 获取当前监听端口
     */
    int getPort() const;

    /**
     * @brief 添加示例路由接口
     * 
     * 添加一组示例路由，展示 GET、POST、PUT、DELETE、PATCH 等 HTTP 方法的使用
     * 包括用户管理 API、健康检查等常见端点
     */
    void addExampleRoutes();
};


#endif // HTTPSERVER_H