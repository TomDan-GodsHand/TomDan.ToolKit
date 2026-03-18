#ifndef HTTPMETHOD_H
#define HTTPMETHOD_H

/**
 * @brief HTTP方法枚举
 * 
 * 定义标准的HTTP请求方法，支持RESTful API设计。
 */
enum class HttpMethod { 
    GET,    /**< 获取资源 */
    POST,   /**< 创建资源 */
    PUT,    /**< 更新资源（完整替换） */
    DELETE, /**< 删除资源 */
    PATCH   /**< 部分更新资源 */
};

#endif // HTTPMETHOD_H