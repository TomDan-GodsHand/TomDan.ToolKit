#ifndef LOCAL_MESSAGEBUS_H
#define LOCAL_MESSAGEBUS_H

#include <map>
#include <string>
#include <vector>
#include <functional>
#include "MessageBus.h"

// 订阅发布模式的消息总线实现
class LocalMessageBus : public MessageBus {
private:
    struct Subscription {
        int id;
        std::string topic;
        std::function<void(const std::string&)> callback;
    };
    
    std::map<std::string, std::vector<int>> topicSubscriptions; // 主题到订阅ID的映射
    std::map<int, Subscription> subscriptions; // 订阅ID到订阅信息的映射
    int nextSubscriptionId = 1;

public:
    void publish(const std::string& topic, const std::string& message) override;
    int subscribe(const std::string& topic, std::function<void(const std::string&)> callback) override;
    void unsubscribe(int subscriptionId) override;
};

#endif // LOCAL_MESSAGEBUS_H