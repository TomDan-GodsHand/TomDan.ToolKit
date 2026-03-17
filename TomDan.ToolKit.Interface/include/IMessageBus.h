#ifndef IMESSAGEBUS_H
#define IMESSAGEBUS_H

#include <string>
#include <functional>

// 信息互通接口 - 订阅发布模式
class IMessageBus {
public:
    virtual ~IMessageBus() = default;
    
    // 发送消息到指定主题
    virtual void publish(const std::string& topic, const std::string& message) = 0;
    
    // 订阅主题，返回订阅ID用于取消订阅
    virtual int subscribe(const std::string& topic, std::function<void(const std::string&)> callback) = 0;
    
    // 取消订阅
    virtual void unsubscribe(int subscriptionId) = 0;
};

#endif // IMESSAGEBUS_H