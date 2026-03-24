# TomDan.ToolKit

一个工具集的核心承载项目，提供插件系统和信息互通接口。

## 项目架构

### 架构设计

TomDan.ToolKit采用分层架构设计，主要包含以下组件：

1. **接口层（TomDan.ToolKit.Interface）**
   - 定义插件接口和消息总线接口
   - 为插件开发提供统一的API规范

2. **核心层（TomDan.ToolKit.Core）**
   - 实现插件管理器，支持动态加载插件
   - 实现消息总线，支持订阅发布模式
   - 提供主程序入口

3. **插件层**
   - 独立开发的插件，编译为动态库（.so文件）
   - 放置在插件目录中，由核心层自动加载

### 项目结构

```
TomDan.ToolKit/
├── TomDan.ToolKit.Interface/    # 接口定义项目
│   ├── include/                 # 接口头文件
│   │   ├── IPlugin.hpp            # 插件接口
│   │   └── IMessageBus.hpp        # 消息总线接口
│   ├── src/                     # 源文件
│   └── xmake.lua                # 构建配置
├── TomDan.ToolKit.Core/         # 核心实现项目
│   ├── include/                 # 内部头文件
│   │   ├── PluginManager.hpp      # 插件管理器
│   │   └── MessageBus.hpp         # 消息总线实现
│   ├── src/                     # 源文件
│   │   ├── main.cpp             # 主程序
│   │   ├── PluginManager.cpp    # 插件管理器实现
│   │   └── MessageBus.cpp       # 消息总线实现
│   └── xmake.lua                # 构建配置
├── build/                       # 构建输出目录
│   └── linux/x86_64/debug/      # 平台和配置相关的构建目录
│       └── plugins/             # 插件目录
├── ExamplePlugin/               # 示例插件项目
│   ├── src/                     # 源文件
│   │   └── ExamplePlugin.cpp    # 示例插件实现
│   └── xmake.lua                # 构建配置
├── xmake.lua                    # 项目管理配置
└── README.md                    # 项目说明文件
```

## 功能特性

### 1. 插件系统

- **动态加载插件**：支持从插件目录加载.so动态库插件
- **插件接口**：定义了统一的插件接口（IPlugin）
- **插件生命周期管理**：支持插件的初始化和执行
- **插件创建和销毁**：通过导出函数createPlugin和destroyPlugin管理插件实例

### 2. 消息总线（订阅发布模式）

- **发布消息**：插件可以发布消息到指定主题
- **订阅消息**：插件可以订阅感兴趣的主题，接收消息通知
- **回调机制**：使用函数回调处理接收到的消息
- **主题管理**：支持多主题消息分发

### 3. 示例插件

- **ExamplePlugin**：演示如何开发独立的插件，包括消息订阅和发布

## 构建和运行

### 使用Xmake构建

```bash
# 在项目根目录执行
xmake

# 运行核心程序
xmake run TomDan.ToolKit.Core
```

### 构建示例插件

```bash
# 构建示例插件
xmake build ExamplePlugin

# 运行核心程序（会自动加载插件）
xmake run TomDan.ToolKit.Core
```

### 使用CMake构建

推荐使用 Clang/LLVM（如果希望使用 libc++），并在项目根目录执行：

```bash
# 在项目根目录
mkdir -p build && cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
# 可执行文件和插件会输出到 build/<系统>/<架构>/<配置>/ 目录
```

### CMake导入第三方库

在CMake中导入第三方库有多种方式，本项目使用`FetchContent`来自动下载和集成asio库：

#### 1. 使用FetchContent（推荐）

在顶层`CMakeLists.txt`中添加：

```cmake
include(FetchContent)

# ASIO standalone library
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-30-2
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(asio)
```

然后在需要使用的子目录中链接：

```cmake
target_include_directories(YourTarget PRIVATE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(YourTarget PRIVATE ASIO_STANDALONE)
```

#### 2. 使用find_package（需要预安装）

如果asio已安装在系统中：

```cmake
find_package(asio REQUIRED)
target_link_libraries(YourTarget PRIVATE asio::asio)
```

#### 3. 使用ExternalProject

对于更复杂的依赖管理：

```cmake
include(ExternalProject)
ExternalProject_Add(asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-30-2
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
```

#### 4. 使用vcpkg或conan

如果使用包管理器：

```bash
# vcpkg
vcpkg install asio

# conan
conan install asio/1.30.2@
```

然后在CMake中：

```cmake
find_package(asio CONFIG REQUIRED)
target_link_libraries(YourTarget PRIVATE asio::asio)
```

## 插件开发指南

### 1. 创建插件项目

1. 创建一个新的目录，如 `MyPlugin`
2. 在目录中创建 `xmake.lua` 文件，配置如下：

```lua
add_rules("mode.debug", "mode.release")

target("MyPlugin")
    set_kind("shared")
    add_defines("PLUGIN_EXPORTS")
    add_includedirs("../TomDan.ToolKit.Interface/include")
    add_files("src/*.cpp")
    set_targetdir("../build/plugins")
```

3. 创建 `src` 目录，并在其中创建插件实现文件

### 2. 实现插件

```cpp
#include "IPlugin.hpp"
#include "IMessageBus.hpp"
#include <iostream>

class MyPlugin : public IPlugin {
private:
    IMessageBus* messageBus;
    int subscriptionId;

public:
    MyPlugin(IMessageBus* bus) : messageBus(bus), subscriptionId(0) {}

    std::string getName() const override {
        return "MyPlugin";
    }

    void initialize() override {
        std::cout << "Initializing " << getName() << std::endl;
        
        // 订阅消息
        subscriptionId = messageBus->subscribe("some_topic", [this](const std::string& message) {
            std::cout << getName() << " received message: " << message << std::endl;
        });
        
        // 发布消息
        messageBus->publish("my_topic", "My plugin initialized");
    }

    void execute() override {
        std::cout << "Executing " << getName() << std::endl;
        // 发布消息
        messageBus->publish("my_topic", "My plugin executed");
    }
};

// 导出函数
extern "C" PLUGIN_API IPlugin* createPlugin(void* context) {
    return new MyPlugin(static_cast<IMessageBus*>(context));
}

extern "C" PLUGIN_API void destroyPlugin(IPlugin* plugin) {
    delete plugin;
}
```

### 3. 构建和部署插件

1. 构建插件：`xmake build MyPlugin`
2. 插件会自动生成到 `TomDan.ToolKit.Core/plugins` 目录
3. 运行核心程序：`xmake run TomDan.ToolKit.Core`，会自动加载插件

## 技术原理

### 动态加载插件

核心程序使用 `dlopen` 和 `dlsym` 函数加载动态库，并通过导出的 `createPlugin` 函数创建插件实例。插件目录默认为 `TomDan.ToolKit.Core/plugins`，核心程序启动时会扫描该目录下的所有 `.so` 文件并尝试加载。

### 订阅发布模式

消息总线维护了主题到订阅者的映射关系，当插件发布消息到某个主题时，消息总线会遍历该主题的所有订阅者，并调用其回调函数传递消息。

### 插件与核心的通信

插件通过构造函数接收消息总线指针，从而可以发布和订阅消息。核心程序在加载插件时，会将消息总线实例传递给插件。

## 未来计划

- **插件依赖管理**：支持插件间的依赖关系管理
- **配置系统**：为插件提供配置文件支持
- **日志系统**：统一的日志记录机制
- **插件版本控制**：支持插件版本管理和兼容性检查
- **跨平台支持**：支持Windows和macOS平台的动态库加载
