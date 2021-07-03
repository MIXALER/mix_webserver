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


class WebServer
{
public:
    WebServer();

    ~WebServer()
    {};

    void init(uint16_t port);

    void start();

public:
    uint16_t port;

};

#endif //MIX_WEBSERVER_WEBSERVER_H
