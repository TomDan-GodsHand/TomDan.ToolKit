#include "HttpServer.h"
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

namespace {
    // 示例控制器：通过宏模拟 C# 的特性/属性来注册路由
    struct ExampleController {
        static std::string getLastPathSegment(const std::string& path) {
            size_t lastSlash = path.find_last_of('/');
            return (lastSlash == std::string::npos ? path : path.substr(lastSlash + 1));
        }

        HTTP_GET(getRoot, "/")
        std::string getRoot(const std::string& /*path*/, const std::string& /*body*/) {
            return "HTTP Server is running! Available endpoints:\n"
                   "GET  /api/hello - Get welcome message\n"
                   "GET  /api/users - Get all users\n"
                   "GET  /api/users/{id} - Get user by ID\n"
                   "POST /api/users - Create new user\n"
                   "PUT  /api/users/{id} - Update user\n"
                   "DELETE /api/users/{id} - Delete user\n"
                   "PATCH /api/users/{id}/status - Update user status";
        }

        HTTP_GET(getHello, "/api/hello")
        std::string getHello(const std::string& /*path*/, const std::string& /*body*/) {
            return "Hello from HTTP Server! Current time: " + std::to_string(time(nullptr));
        }

        HTTP_GET(getUsers, "/api/users")
        std::string getUsers(const std::string& /*path*/, const std::string& /*body*/) {
            return "[\n"
                   "  {\"id\": 1, \"name\": \"Alice\", \"email\": \"alice@example.com\"},\n"
                   "  {\"id\": 2, \"name\": \"Bob\", \"email\": \"bob@example.com\"},\n"
                   "  {\"id\": 3, \"name\": \"Charlie\", \"email\": \"charlie@example.com\"}\n"
                   "]";
        }

        HTTP_GET(getUserById, "/api/users/:id")
        std::string getUserById(const std::string& path, const std::string& /*body*/) {
            std::string idStr = getLastPathSegment(path);
            int id = std::stoi(idStr);

            if (id == 1) {
                return "{\"id\": 1, \"name\": \"Alice\", \"email\": \"alice@example.com\", \"status\": \"active\"}";
            } else if (id == 2) {
                return "{\"id\": 2, \"name\": \"Bob\", \"email\": \"bob@example.com\", \"status\": \"active\"}";
            } else {
                return "{\"error\": \"User not found\", \"id\": " + idStr + "}";
            }
        }

        HTTP_POST(createUser, "/api/users")
        std::string createUser(const std::string& /*path*/, const std::string& body) {
            if (body.empty()) {
                return "{\"error\": \"Request body is required\"}";
            }
            return "{\"message\": \"User created successfully\", \"data\": " + body + ", \"id\": 100}";
        }

        HTTP_PUT(updateUser, "/api/users/:id")
        std::string updateUser(const std::string& path, const std::string& body) {
            if (body.empty()) {
                return "{\"error\": \"Request body is required for update\"}";
            }
            std::string idStr = getLastPathSegment(path);
            return "{\"message\": \"User updated successfully\", \"id\": " + idStr + ", \"data\": " + body + "}";
        }

        HTTP_DELETE(deleteUser, "/api/users/:id")
        std::string deleteUser(const std::string& path, const std::string& /*body*/) {
            std::string idStr = getLastPathSegment(path);
            return "{\"message\": \"User deleted successfully\", \"id\": " + idStr + "}";
        }

        HTTP_PATCH(patchUserStatus, "/api/users/:id/status")
        std::string patchUserStatus(const std::string& path, const std::string& body) {
            if (body.empty()) {
                return "{\"error\": \"Status data is required\"}";
            }
            std::string idStr = getLastPathSegment(path);
            return "{\"message\": \"User status updated\", \"id\": " + idStr + ", \"status\": " + body + "}";
        }

        HTTP_GET(getHealth, "/health")
        std::string getHealth(const std::string& /*path*/, const std::string& /*body*/) {
            return "{\"status\": \"healthy\", \"timestamp\": " + std::to_string(time(nullptr)) + "}";
        }

        HTTP_GET(getServerInfo, "/api/server/info")
        std::string getServerInfo(const std::string& /*path*/, const std::string& /*body*/) {
            return "{\"name\": \"TomDan HTTP Server\", \"version\": \"1.0.0\", \"framework\": \"cpp-httplib\"}";
        }

        void registerRoutes(HttpServer* server) {
            REGISTER_ROUTE(GET, getRoot);
            REGISTER_ROUTE(GET, getHello);
            REGISTER_ROUTE(GET, getUsers);
            REGISTER_ROUTE(GET, getUserById);
            REGISTER_ROUTE(POST, createUser);
            REGISTER_ROUTE(PUT, updateUser);
            REGISTER_ROUTE(DELETE, deleteUser);
            REGISTER_ROUTE(PATCH, patchUserStatus);
            REGISTER_ROUTE(GET, getHealth);
            REGISTER_ROUTE(GET, getServerInfo);
        }
    };
}

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

    addExampleRoutes();
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


// 添加示例接口
void HttpServer::addExampleRoutes() {
    // GET 示例：获取欢迎信息
    addRoute(HttpMethod::GET, "/", [](const std::string& path, const std::string& body) -> std::string {
        return "HTTP Server is running! Available endpoints:\n"
               "GET  /api/hello - Get welcome message\n"
               "GET  /api/users - Get all users\n"
               "GET  /api/users/{id} - Get user by ID\n"
               "POST /api/users - Create new user\n"
               "PUT  /api/users/{id} - Update user\n"
               "DELETE /api/users/{id} - Delete user\n"
               "PATCH /api/users/{id}/status - Update user status";
    });

    // GET 示例：获取欢迎消息
    addRoute(HttpMethod::GET, "/api/hello", [](const std::string& path, const std::string& body) -> std::string {
        return "Hello from HTTP Server! Current time: " + std::to_string(time(nullptr));
    });

    // GET 示例：获取所有用户
    addRoute(HttpMethod::GET, "/api/users", [](const std::string& path, const std::string& body) -> std::string {
        return "[\n"
               "  {\"id\": 1, \"name\": \"Alice\", \"email\": \"alice@example.com\"},\n"
               "  {\"id\": 2, \"name\": \"Bob\", \"email\": \"bob@example.com\"},\n"
               "  {\"id\": 3, \"name\": \"Charlie\", \"email\": \"charlie@example.com\"}\n"
               "]";
    });

    // GET 示例：通过ID获取用户（使用正则表达式路径）
    addRoute(HttpMethod::GET, "/api/users/:id", [](const std::string& path, const std::string& body) -> std::string {
        // 从路径中提取ID
        size_t lastSlash = path.find_last_of('/');
        std::string idStr = path.substr(lastSlash + 1);
        int id = std::stoi(idStr);
        
        if (id == 1) {
            return "{\"id\": 1, \"name\": \"Alice\", \"email\": \"alice@example.com\", \"status\": \"active\"}";
        } else if (id == 2) {
            return "{\"id\": 2, \"name\": \"Bob\", \"email\": \"bob@example.com\", \"status\": \"active\"}";
        } else {
            return "{\"error\": \"User not found\", \"id\": " + idStr + "}";
        }
    });

    // POST 示例：创建新用户
    addRoute(HttpMethod::POST, "/api/users", [](const std::string& path, const std::string& body) -> std::string {
        if (body.empty()) {
            return "{\"error\": \"Request body is required\"}";
        }
        
        // 模拟创建用户逻辑
        return "{\"message\": \"User created successfully\", \"data\": " + body + ", \"id\": 100}";
    });

    // PUT 示例：更新用户
    addRoute(HttpMethod::PUT, "/api/users/:id", [](const std::string& path, const std::string& body) -> std::string {
        if (body.empty()) {
            return "{\"error\": \"Request body is required for update\"}";
        }
        
        // 从路径中提取ID
        size_t lastSlash = path.find_last_of('/');
        std::string idStr = path.substr(lastSlash + 1);
        
        return "{\"message\": \"User updated successfully\", \"id\": " + idStr + ", \"data\": " + body + "}";
    });

    // DELETE 示例：删除用户
    addRoute(HttpMethod::DELETE, "/api/users/:id", [](const std::string& path, const std::string& body) -> std::string {
        // 从路径中提取ID
        size_t lastSlash = path.find_last_of('/');
        std::string idStr = path.substr(lastSlash + 1);
        
        return "{\"message\": \"User deleted successfully\", \"id\": " + idStr + "}";
    });

    // PATCH 示例：部分更新用户状态
    addRoute(HttpMethod::PATCH, "/api/users/:id/status", [](const std::string& path, const std::string& body) -> std::string {
        if (body.empty()) {
            return "{\"error\": \"Status data is required\"}";
        }
        
        // 从路径中提取ID
        size_t lastSlash = path.find_last_of('/');
        std::string idStr = path.substr(0, lastSlash);
        lastSlash = idStr.find_last_of('/');
        idStr = idStr.substr(lastSlash + 1);
        
        return "{\"message\": \"User status updated\", \"id\": " + idStr + ", \"status\": " + body + "}";
    });

    // 健康检查端点
    addRoute(HttpMethod::GET, "/health", [](const std::string& path, const std::string& body) -> std::string {
        return "{\"status\": \"healthy\", \"timestamp\": " + std::to_string(time(nullptr)) + "}";
    });

    // 获取服务器信息
    addRoute(HttpMethod::GET, "/api/server/info", [](const std::string& path, const std::string& body) -> std::string {
        return "{\"name\": \"TomDan HTTP Server\", \"version\": \"1.0.0\", \"framework\": \"cpp-httplib\"}";
    });

    std::cout << "[HttpServer] Example routes added successfully" << std::endl;
}
