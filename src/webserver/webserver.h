//
// Created by yuanh on 2021/4/2.
//

#ifndef MIX_WEBSERVER_WEBSERVER_H
#define MIX_WEBSERVER_WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "../http/http_conn.h"
#include "../thread_pool/thread_pool.h"
#include "../util/util.h"

const int kMaxFd = 65536;
const int kMaxEventNumber = 10000;
const int kTimeSlot = 5;

class WebServer
{
public:
    WebServer();

    ~WebServer();

    void
    Init(uint16_t port, string user, string passwd, string database_name, int log_write, int opt_linger, int trig_mode,
         int sql_num, int thread_num, int close_log, int actor_mode);

    void ThreadPool();

    void SqlPool();

    void LogWrite();

    void TrigMode();

    void EventListen();

    void EventLoop();

    void Timer(int conn_fd, struct sockaddr_in client_addr);

    void AdjustTimer(UtilTimer *timer);

    void DealTimer(UtilTimer *timer, int sock_fd);

    bool DealClientData();

    bool DealWithSignal(bool &timeout, bool &stop_server);

    void DealWithThread(int sock_fd);

    void DealWithWrite(int sock_fd);

public:
    uint16_t port_;
    char *root_;
    int log_write_;
    int close_log_;
    int actor_mode_;

    int pipe_fd_[2];//管道
    int epoll_fd_;
    HttpConn *users_;
    // 数据库相关
    ConnectionPool *conn_pool_;
    string user_;
    string passwd_;
    string database_name_;
    int sql_num_;

    int thread_num_;
    ThreadP<HttpConn> *pool_;

    epoll_event events_[kMaxEventNumber];

    int listen_fd_;

    int opt_linger_;

    int trig_mode_;

    int listen_trig_mode_;
    int conn_trig_mode_;

    ClientData *user_timer_;
    Util utils_;
};

#endif //MIX_WEBSERVER_WEBSERVER_H
