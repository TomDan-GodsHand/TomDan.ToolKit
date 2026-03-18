## 路线图：基于 DHT 和 UDP 的分布式 MessageBus 扩展

**TL;DR** — 将分布式 P2P 通信能力直接集成到现有 MessageBus 中，使其同时具备本地内核与网络层，使用 UDP 传输和 DHT（Kademlia）进行节点发现与主题路由，保持与现有插件的二进制兼容性。

### 阶段 1：架构设计与基础设置
1. **设计增强的 MessageBus 架构**（*并行*）
   - 定义 `INetworkTransport` 接口，支持 UDP（可靠/不可靠）和 TCP 后备，实现节点发现、路由维护、NAT 穿透
   - 定义 `IDHT` 接口，包含节点路由、键值存储、活性检测，用于分布式主题路由
   - 设计订阅同步模块，负责本地订阅信息与 DHT 的同步
   - 起草协议消息格式（JSON 或 Protobuf）
2. **在 Core 模块中扩展目录结构**（*依赖步骤1*）
   - 在 `TomDan.ToolKit.Core/include/network/` 中添加网络层头文件
   - 在 `TomDan.ToolKit.Core/include/dht/` 中添加 DHT 头文件
   - 在 `TomDan.ToolKit.Core/src/network/` 和 `TomDan.ToolKit.Core/src/dht/` 中添加实现文件
   - 更新 `TomDan.ToolKit.Core/xmake.lua` 添加网络库依赖
   - 评估并添加网络库依赖（如 Asio 或自定义 socket 封装）

### 阶段 2：网络传输层实现
3. **实现 UDP 基础传输**（*依赖阶段1*）
   - 编写 `UDPTransport` 类，实现 `INetworkTransport`
   - 支持数据报发送/接收、端点管理
   - 实现简单的应用层 ACK/重传机制（可选可靠性）
   - 集成 NAT 穿透辅助（STUN/UPnP），支持内网节点互通
4. **实现消息分片与重组**（*依赖步骤3*）
   - 对于超大数据，支持 UDP 分片/重组
   - 实现协商机制，当数据超限时自动切换 TCP 或分片 UDP
5. **实现 TCP 后备传输**（*并行步骤3*）
   - 编写 `TCPTransport` 类，用于可靠流传输
   - 集成到同一 `INetworkTransport` 接口下

### 阶段 3：DHT 实现
6. **实现 Kademlia 核心**（*依赖阶段1*）
   - 实现 `NodeID`、`RoutingTable`、`KBucket`
   - 实现 `find_node`、`find_value`、`store` 远程过程
   - 实现节点活性检测（Ping/Pong）
7. **集成键值存储**（*依赖步骤6*）
   - 使用 DHT 存储主题订阅映射（主题 → 节点列表）
   - 实现主题的发布/订阅信息在 DHT 中的分布式存储
8. **实现节点发现与加入**（*依赖步骤6*）
   - 实现引导过程（已知种子节点）
   - 自动维护路由表，处理节点加入/离开

### 阶段 4：增强 MessageBus 实现
9. **增强 MessageBus 类**（*依赖阶段2、3*）
   - 在现有 `MessageBus` 类中增加 `INetworkTransport` 和 `IDHT` 实例成员，保持 `IMessageBus` 接口不变
   - 重写 `publish`：先处理本地订阅，再通过 DHT 查找远程订阅节点，通过 UDP 转发消息
   - 重写 `subscribe`：将本地订阅注册到本地内核，同时通过订阅同步模块将订阅信息发布到 DHT
   - 重写 `unsubscribe`：移除本地订阅，并通过订阅同步模块从 DHT 移除订阅信息
   - 实现本地消息总线内核，维护本地订阅映射与回调
10. **实现订阅同步与消息路由**（*依赖步骤9*）
    - 实现订阅同步模块，负责将本地订阅信息同步到 DHT，并从 DHT 获取其他节点的订阅信息
    - 监听 DHT 中主题订阅的变化，更新本地路由缓存
    - 处理节点加入/离开时的订阅重新分配
    - 定义消息信封格式，包含源节点、主题、消息体、TTL
    - 实现跨节点消息转发，避免循环
    - 添加本地消息直接交付优化

### 阶段 5：测试与集成
11. **单元测试**（*并行各阶段*）
    - 为每个新组件编写单元测试（网络传输、DHT、总线逻辑）
    - 使用模拟网络环境测试分布式场景
12. **集成现有插件**（*依赖步骤9*）
    - 修改 `main.cpp` 初始化增强的 `MessageBus`（配置网络参数与 DHT 种子节点）
    - 验证 ExamplePlugin 和 HttpServerPlugin 无需修改即可工作
13. **跨节点部署测试**（*依赖步骤12*）
    - 编写多节点测试程序，模拟跨互联网十几个节点
    - 测试主题订阅、消息发布、节点故障恢复

### 阶段 6：优化与文档
14. **性能优化**（*依赖阶段5*）
    - 优化 DHT 查找延迟、UDP 吞吐量
    - 实现消息缓存、批量发送
15. **编写文档**（*并行*）
    - API 文档、部署指南、配置说明
    - 示例代码展示分布式总线用法

### 相关文件
- `TomDan.ToolKit.Interface/include/IMessageBus.h` — 现有接口，无需修改（二进制兼容）
- `TomDan.ToolKit.Core/include/network/INetworkTransport.h` — 新接口
- `TomDan.ToolKit.Core/include/dht/IDHT.h` — 新接口
- `TomDan.ToolKit.Core/include/MessageBus.h` — 增强现有类，增加网络与 DHT 成员
- `TomDan.ToolKit.Core/src/MessageBus.cpp` — 增强实现，集成分布式发布/订阅逻辑
- `TomDan.ToolKit.Core/src/network/` — 网络传输实现
- `TomDan.ToolKit.Core/src/dht/` — DHT 实现
- `TomDan.ToolKit.Core/src/main.cpp` — 修改 MessageBus 初始化，配置网络参数与 DHT 种子节点
- `TomDan.ToolKit.Core/xmake.lua` — 添加网络库依赖

### 验证步骤
1. 编译 Core 模块，确保无编译错误（包含新的分布式组件）
2. 运行单元测试，覆盖率 >80%
3. 启动两个节点，互相发现，订阅相同主题，验证消息跨节点传递
4. 运行现有插件（ExamplePlugin、HttpServerPlugin），确认功能正常
5. 模拟节点宕机，验证订阅恢复与消息重新路由

### 决策与假设
- **传输协议**：以 UDP 为主，应用层实现可靠性；大数据协商切换 TCP
- **DHT 协议**：采用 Kademlia，因其广泛使用、简单且适合 P2P 发现
- **兼容性**：保持 `IMessageBus` 接口不变，插件零修改
- **节点规模**：设计支持数十个节点，路由表大小适中
- **安全性**：暂不实现加密/认证（可后续扩展）

### 后续考虑
1. 是否需要支持多播/广播用于本地节点发现？
2. 是否添加管理 API（如节点状态查询）？
3. 是否预留扩展点以支持其他 DHT 协议？