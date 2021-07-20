//
// Created by yuanh on 2021/4/3.
//

#ifndef WEBSERVER_LIST_TIMER_H
#define WEBSERVER_LIST_TIMER_H

#include "string.h"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <time.h>

#include "../util/util.h"
#include "../http/http_conn.h"

class UtilTimer;

struct ClientData
{
    sockaddr_in address_;
    int sock_fd_;
    UtilTimer *timer_;
};

class UtilTimer
{
public:
    UtilTimer() : prev_(NULL), next_(NULL)
    {}

public:
    time_t expire_;

    void (*cb_func_)(ClientData *);

    ClientData *user_data_;

    UtilTimer *prev_;
    UtilTimer *next_;
};

class SortTimerList
{
public:
    SortTimerList();

    ~SortTimerList();

    void AddTimer(UtilTimer *timer);

    void AdjustTimer(UtilTimer *timer);

    void DelTimer(UtilTimer *timer);

    void Tick();

private:
    void AddTimer(UtilTimer *timer, UtilTimer *list_head);

    UtilTimer *head_;
    UtilTimer *tail_;
};

void CbFunc(ClientData *user_data);

#endif //WEBSERVER_LIST_TIMER_H
