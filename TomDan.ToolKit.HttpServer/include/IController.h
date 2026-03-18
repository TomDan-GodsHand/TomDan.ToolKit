#ifndef ICONTROLLER_H
#define ICONTROLLER_H

#include <string>

// 前向声明
class HttpServer;

/**
 * @brief 控制器接口类
 * 
 * 所有控制器都应该继承这个接口，提供统一的路由注册方法。
 */
class IController {
public:
    virtual ~IController() = default;
    
    /**
     * @brief 注册控制器中的所有路由到 HttpServer
     * @param server HttpServer 实例
     */
    virtual void registerRoutes(HttpServer* server) = 0;
    
    /**
     * @brief 获取控制器名称（用于日志和调试）
     * @return 控制器名称
     */
    virtual std::string getName() const = 0;
};

#endif // ICONTROLLER_H