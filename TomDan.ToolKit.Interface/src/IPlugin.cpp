#include "IPlugin.h"

IPlugin::~IPlugin() = default;

// 可选：提供 setPath 的默认实现
std::string IPlugin::setPath(const std::string &path) {
    // 默认实现不执行任何操作，返回空字符串
    return "";
}