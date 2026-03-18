# HTTP Server Controller 架构

## 概述

新的Controller架构将路由处理逻辑从HttpServer类中分离出来，提供了更清晰、更模块化的代码结构。Controller架构支持以下特性：

1. **模块化设计**：每个Controller处理一组相关的路由
2. **代码复用**：Controller可以在不同的服务器实例中重用
3. **易于测试**：Controller可以独立于HTTP服务器进行测试
4. **类型安全**：使用C++类和宏提供类型安全的路由注册

## 文件结构

```
TomDan.ToolKit.HttpServer/
├── include/
│   ├── HttpServer.h          # HTTP服务器主类
│   ├── IController.h         # 控制器接口
│   ├── ExampleController.h   # 示例控制器
│   └── ControllerManager.h   # 控制器管理器
├── src/
│   ├── HttpServer.cpp        # HTTP服务器实现
│   ├── ExampleController.cpp # 示例控制器实现
│   └── HttpServerPlugin.cpp  # HTTP服务器插件
└── example/
    └── ControllerExample.cpp # 控制器使用示例
```

## 核心组件

### 1. IController (控制器接口)

所有控制器都应该继承这个接口：

```cpp
class IController {
public:
    virtual ~IController() = default;
    virtual void registerRoutes(HttpServer* server) = 0;
    virtual std::string getName() const = 0;
};
```

### 2. HTTP_ROUTE 宏

提供类似C# Attribute风格的路由注册：

```cpp
// 定义路由
HTTP_GET(getUsers, "/api/users")
std::string getUsers(const std::string& path, const std::string& body);

// 注册路由（在registerRoutes方法中调用）
REGISTER_ROUTE(GET, getUsers);
```

支持的HTTP方法：
- `HTTP_GET(name, path)`
- `HTTP_POST(name, path)`
- `HTTP_PUT(name, path)`
- `HTTP_DELETE(name, path)`
- `HTTP_PATCH(name, path)`

### 3. ExampleController (示例控制器)

展示完整的控制器实现，包括：
- 用户管理API
- 健康检查端点
- 服务器信息端点

### 4. ControllerManager (控制器管理器)

用于管理多个控制器：

```cpp
ControllerManager manager;
manager.addController(std::make_unique<ExampleController>());
manager.addController(std::make_unique<UserController>());
manager.registerAllRoutes(&server);
```

## 使用方法

### 方法1：直接注册控制器

```cpp
HttpServer server(8080);
ExampleController controller;
server.registerController(&controller);
server.start();
```

### 方法2：使用ControllerManager

```cpp
HttpServer server(8080);
ControllerManager manager;

// 添加多个控制器
manager.addController(std::make_unique<ExampleController>());
manager.addController(std::make_unique<UserController>());
manager.addController(std::make_unique<ProductController>());

// 注册所有路由
manager.registerAllRoutes(&server);
server.start();
```

### 方法3：创建自定义控制器

```cpp
class UserController : public IController {
public:
    HTTP_GET(getAllUsers, "/api/users")
    std::string getAllUsers(const std::string& path, const std::string& body) {
        return "{\"users\": [...]}";
    }
    
    HTTP_POST(createUser, "/api/users")
    std::string createUser(const std::string& path, const std::string& body) {
        return "{\"message\": \"User created\"}";
    }
    
    void registerRoutes(HttpServer* server) override {
        REGISTER_ROUTE(GET, getAllUsers);
        REGISTER_ROUTE(POST, createUser);
    }
    
    std::string getName() const override {
        return "UserController";
    }
};
```

## 路径参数处理

控制器方法可以处理路径参数：

```cpp
HTTP_GET(getUserById, "/api/users/:id")
std::string getUserById(const std::string& path, const std::string& body) {
    // 从路径中提取ID
    size_t lastSlash = path.find_last_of('/');
    std::string idStr = path.substr(lastSlash + 1);
    return "{\"id\": " + idStr + ", \"name\": \"User " + idStr + "\"}";
}
```

## 最佳实践

1. **单一职责**：每个控制器应该只处理一组相关的路由
2. **命名规范**：控制器名称应该以"Controller"结尾
3. **错误处理**：在控制器方法中进行适当的错误处理
4. **日志记录**：在registerRoutes方法中添加日志记录
5. **资源管理**：使用智能指针管理控制器生命周期

## 示例运行

编译并运行示例程序：

```bash
cd /home/tomdan/repos/TomDan.ToolKit
xmake build TomDan.ToolKit.HttpServer
./build/linux/x86_64/debug/bin/HttpServerExample
```

或者运行控制器示例：

```bash
cd TomDan.ToolKit.HttpServer
g++ -std=c++17 -I./include -I../TomDan.ToolKit.Interface/include example/ControllerExample.cpp src/*.cpp -lhttplib -lpthread -o controller_example
./controller_example
```

## 扩展性

新的Controller架构易于扩展：

1. **中间件支持**：可以在Controller中添加中间件处理
2. **依赖注入**：可以通过构造函数注入依赖
3. **配置管理**：可以从外部配置文件加载路由配置
4. **自动发现**：可以实现控制器的自动发现和注册