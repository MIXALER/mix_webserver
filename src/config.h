//
// Created by yuanh on 2021/7/2.
//

#ifndef MIX_WEBSERVER_CONFIG_H
#define MIX_WEBSERVER_CONFIG_H


#include "webserver.h"

using namespace std;

class Config
{
public:
    Config();

    ~Config()
    {};

    void ParseConfig(int argc, char **argv);

    // 端口号
    uint16_t port_;
    // 日志写入方式
    int log_write_;
    // 触发组合模式
    int trig_mode_;
    // listenfd 触发模式
    int listen_fd_trig_mode_;
    //connfd 触发模式
    int conn_trig_mode_;
    // 优雅关闭链接
    int opt_linger_;
    // 数据库连接池数量
    int sql_num_;
    // 线程池线程数量
    int thread_num_;
    // 是否关闭日志；
    int close_log_;
    // 并发模型选择
    int actor_mode_;
};


#endif //MIX_WEBSERVER_CONFIG_H
