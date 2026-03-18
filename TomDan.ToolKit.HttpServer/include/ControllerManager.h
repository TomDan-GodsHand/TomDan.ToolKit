#ifndef CONTROLLERMANAGER_H
#define CONTROLLERMANAGER_H

#include "IController.h"
#include <vector>
#include <memory>
#include <iostream>

/**
 * @brief 控制器管理器
 * 
 * 用于管理多个控制器，统一注册所有控制器的路由。
 */
class ControllerManager {
private:
    std::vector<std::unique_ptr<IController>> controllers;
    
public:
    ControllerManager() = default;
    
    /**
     * @brief 添加控制器
     * @param controller 控制器指针（所有权将被转移）
     */
    void addController(std::unique_ptr<IController> controller) {
        if (controller) {
            controllers.push_back(std::move(controller));
        }
    }
    
    /**
     * @brief 注册所有控制器的路由
     * @param server HttpServer 实例
     */
    void registerAllRoutes(HttpServer* server) {
        if (!server) return;
        
        std::cout << "[ControllerManager] Registering routes from " << controllers.size() << " controllers" << std::endl;
        
        for (const auto& controller : controllers) {
            std::cout << "[ControllerManager] Registering routes from " << controller->getName() << std::endl;
            controller->registerRoutes(server);
        }
        
        std::cout << "[ControllerManager] All routes registered successfully" << std::endl;
    }
    
    /**
     * @brief 获取控制器数量
     * @return 控制器数量
     */
    size_t getControllerCount() const {
        return controllers.size();
    }
    
    /**
     * @brief 清空所有控制器
     */
    void clear() {
        controllers.clear();
    }
};

#endif // CONTROLLERMANAGER_H