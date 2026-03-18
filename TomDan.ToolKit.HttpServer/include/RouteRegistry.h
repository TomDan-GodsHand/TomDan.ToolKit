#ifndef ROUTEREGISTRY_H
#define ROUTEREGISTRY_H

#include "HttpMethod.h"
#include <string>
#include <functional>

/**
 * @brief 路由信息结构体
 * 
 * 包含路由的HTTP方法、路径和处理函数。
 * 用于存储路由元数据，可供路由监控、文档生成等工具使用。
 */
struct RouteInfo {
    HttpMethod method;
    std::string path;
    std::function<std::string(const std::string&, const std::string&)> handler;
};

#endif // ROUTEREGISTRY_H