//
// Created by yuanh on 2021/4/19.
//

#ifndef MIX_WEBSERVER_UTIL_H
#define MIX_WEBSERVER_UTIL_H

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
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <time.h>

#include "../log/log.h"
#include "../timer/list_timer.h"

class UtilTimer;

class SortTimerList;

class Util
{
public:
    Util()
    {}

    ~Util()
    {}

    void Init(int timeslot);

    int SetNoBlocking(int fd);

    void AddFd(int epoll_fd, int fd, bool one_shot, int trig_mode);

    static void SigHandler(int sig);

    void AddSig(int sig, void(handler)(int), bool restart = true);

    void TimerHandler();

    void ShowError(int conn_fd, const char *info);

public:
    static int *pipe_fd_;
    SortTimerList timer_list_;
    static int epoll_fd_;
    int timeslot_;
};

#endif //MIX_WEBSERVER_UTIL_H
