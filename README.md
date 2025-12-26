# 基于Reactor模式的高性能网络服务器

## 项目概述

本项目实现了一个基于Reactor模式的高性能网络服务器，采用多线程事件循环模型，支持并发处理多个客户端连接。服务器使用epoll作为I/O多路复用机制，结合非阻塞I/O操作，能够高效处理网络事件。

## 核心架构

服务器采用Reactor模式设计，主要包含以下核心组件：

1. **EventLoop**：事件循环，负责监听和分发I/O事件
2. **Event**：封装文件描述符及其相关事件和回调函数
3. **Server**：服务器主类，负责初始化监听套接字和启动服务
4. **ReactorThreadPool**：Reactor线程池，管理多个事件循环线程
5. **ReactorThread**：线程池中的工作线程，每个线程运行一个EventLoop
6. **Handler**：事件处理器，实现连接建立、数据接收和发送的具体逻辑

## 工作原理

1. 主程序初始化一个基础EventLoop和Reactor线程池
2. Server创建监听套接字并注册到基础EventLoop
3. 当有新连接到来时，Accept事件被触发，由handleAccept处理
4. 新连接被分配到线程池中的某个EventLoop（轮询方式）
5. 每个连接的读写事件由对应的EventLoop处理，并通过Handler中的回调函数完成实际的数据处理
6. 采用边缘触发(EPOLLET)模式提高I/O效率

## 使用方法

1. 编译项目（需支持C++11及以上标准）
   ```bash
   g++ -std=c++11 *.cpp -o reactor_server -lpthread
   ```

2. 运行服务器
   ```bash
   ./reactor_server
   ```
   默认监听地址为192.168.80.128:10000，可在main.cpp中修改

3. 测试连接
   使用telnet或其他客户端工具连接服务器进行测试
   ```bash
   telnet 192.168.80.128 10000
   ```

## 主要特性

- 基于epoll的I/O多路复用
- 多线程事件循环，充分利用多核CPU
- 非阻塞I/O操作，提高吞吐量
- 边缘触发模式，减少事件触发次数
- 线程安全的事件操作
- 支持TCP连接的建立、数据接收和发送

