#ifndef EXAMPLECONTROLLER_H
#define EXAMPLECONTROLLER_H

#include "HttpServer.h"
#include "IController.h"
#include <string>

/**
 * @brief 示例控制器类
 * 
 * 展示如何使用 HTTP_ROUTE 宏注册路由，提供用户管理、健康检查等示例接口。
 * 通过宏模拟 C# 的特性/属性风格的路由注册。
 */
class ExampleController : public IController {
public:
    /**
     * @brief 获取路径的最后一段（用于提取路径参数）
     * @param path 请求路径
     * @return 路径的最后一段
     */
    static std::string getLastPathSegment(const std::string& path);

    // 路由定义
    HTTP_GET(getRoot, "/")
    std::string getRoot(const std::string& path, const std::string& body);

    HTTP_GET(getHello, "/api/hello")
    std::string getHello(const std::string& path, const std::string& body);

    HTTP_GET(getUsers, "/api/users")
    std::string getUsers(const std::string& path, const std::string& body);

    HTTP_GET(getUserById, "/api/users/:id")
    std::string getUserById(const std::string& path, const std::string& body);

    HTTP_POST(createUser, "/api/users")
    std::string createUser(const std::string& path, const std::string& body);

    HTTP_PUT(updateUser, "/api/users/:id")
    std::string updateUser(const std::string& path, const std::string& body);

    HTTP_DELETE(deleteUser, "/api/users/:id")
    std::string deleteUser(const std::string& path, const std::string& body);

    HTTP_PATCH(patchUserStatus, "/api/users/:id/status")
    std::string patchUserStatus(const std::string& path, const std::string& body);

    HTTP_GET(getHealth, "/health")
    std::string getHealth(const std::string& path, const std::string& body);

    HTTP_GET(getServerInfo, "/api/server/info")
    std::string getServerInfo(const std::string& path, const std::string& body);

    /**
     * @brief 注册所有路由到 HttpServer
     * @param server HttpServer 实例
     */
    void registerRoutes(HttpServer* server) override;
    
    /**
     * @brief 获取控制器名称（用于日志和调试）
     * @return 控制器名称
     */
    std::string getName() const override;
};

#endif // EXAMPLECONTROLLER_H