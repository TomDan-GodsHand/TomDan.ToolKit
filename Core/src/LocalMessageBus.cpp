#include "LocalMessageBus.hpp"
#include <iostream>
#include <algorithm>

void LocalMessageBus::publish(const std::string& topic, const std::string& message) {
    std::cout << "Publishing to topic '" << topic << "': " << message << std::endl;
    
    // 通知所有订阅者
    auto it = topicSubscriptions.find(topic);
    if (it != topicSubscriptions.end()) {
        for (int subscriptionId : it->second) {
            auto subIt = subscriptions.find(subscriptionId);
            if (subIt != subscriptions.end()) {
                subIt->second.callback(message);
            }
        }
    }
}

int LocalMessageBus::subscribe(const std::string& topic, std::function<void(const std::string&)> callback) {
    int id = nextSubscriptionId++;
    
    // 创建订阅
    Subscription subscription;
    subscription.id = id;
    subscription.topic = topic;
    subscription.callback = callback;
    
    // 添加到订阅映射
    subscriptions[id] = subscription;
    
    // 添加到主题订阅列表
    topicSubscriptions[topic].push_back(id);
    
    std::cout << "Subscribed to topic '" << topic << "' with ID: " << id << std::endl;
    return id;
}

void LocalMessageBus::unsubscribe(int subscriptionId) {
    auto it = subscriptions.find(subscriptionId);
    if (it != subscriptions.end()) {
        std::string topic = it->second.topic;
        
        // 从主题订阅列表中移除
        auto topicIt = topicSubscriptions.find(topic);
        if (topicIt != topicSubscriptions.end()) {
            auto& ids = topicIt->second;
            ids.erase(std::remove(ids.begin(), ids.end(), subscriptionId), ids.end());
        }
        
        // 从订阅映射中移除
        subscriptions.erase(it);
        
        std::cout << "Unsubscribed from topic with ID: " << subscriptionId << std::endl;
    }
}