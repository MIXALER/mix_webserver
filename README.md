# WebServer

Linux 环境下 C++ 轻量级 Web 服务器。用于学习 Linux 环境网络编程。

-   使用线程池 + 非阻塞 socket + epoll（ET 和 LT 均实现）+ 事件处理（Reactor 和模拟 Proactor 均实现）的并发模型
-   使用状态机解析 HTTP 请求报文，支持解析 GET 和 POST 请求
-   访问服务器数据库实现 Web 端用户注册，登录功能，可以请求服务器图片和视频文件
-   实现同步/异步日志系统，记录服务器运行状态
-   经 Webbench 压力测试，可以实现上万的并发连接和数据请求

## 1 基础知识

高性能服务器程序框架

I/O 模型

Reactor 事件处理模式

I/O 复用

项目整体介绍

## 2 开发环境

ubuntu 16，gcc，cmake，clion

## 3 设计框架



## 4 代码解读

[线程同步机制封装](docs/线程同步机制类封装.md)

http 请求解析类

[半同步/半反应堆线程池](docs/线程池设计与实现.md)

阻塞队列

日志类

webserver 类

## 5 压力测试

## 6 更新日志

-   [ ] 完成配置系统

## 7 快速运行

# 参考资料

[游双 《Linux 高性能服务器编程》](https://book.douban.com/subject/24722611/)

