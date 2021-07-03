//
// Created by yuanh on 2021/7/2.
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

#include "http/http_conn.h"


class WebServer
{
public:
    WebServer();

    ~WebServer()
    {};

public:
    uint16_t port;
    char* root_;
    int log_write_;
    int close_log_;
    int actor_mode_;

    int pipe_fd_[2];
    int epoll_fd_;


};

#endif //MIX_WEBSERVER_WEBSERVER_H
