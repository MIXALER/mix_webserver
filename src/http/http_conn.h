//
// Created by yuanh on 2021/7/3.
//

#ifndef WEBSERVER_HTTP_CONN_H
#define WEBSERVER_HTTP_CONN_H

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
#include <map>

#include "lock/locker.h"
#include "mysql/sql_conn_pool.h"
#include "timer/list_timer.h"
#include "log/log.h"

using namespace std;

class HttpConn
{
public:
    static const int kFileNameLen = 200;
    static const int kReadBufferSize = 2048;
    static const int KWriteBufferSize = 1024;
    enum Method
    {
        kGet = 0,
        kPost,
        kHead,
        kPut,
        kDelete,
        kTrace,
        kOptions,
        kConnect,
        kPath
    };
    enum CheckState
    {
        kCheckStateRequestLine = 0,
        kCheckStateHeader,
        kCheckStateContent
    };
    enum HttpCode
    {
        kNoRequest,
        kGetRequest,
        kBadRequest,
        kNoResource,
        kForbiddenRequest,
        kFileRequest,
        kInternalError,
        kClosedConnection
    };
    enum LineStatus
    {
        kLineOk = 0,
        kLineBad,
        kLineOpen
    };
public:
    HttpConn()
    {}

    ~HttpConn()
    {}

public:
    void
    Init(int sock_fd, const sockaddr_in &addr, char *root, int trig_mode, int close_log, string user, string passwd,
         string sql_name);

    void ThreadPool();

    void SqlPool();

    void LogWrite();

    void TrigMode();

    void EventListen();

    void EventLoop();

    void Timer(int conn_fd, struct sockaddr_in client_address);

    void Adjust_timer()
};

#endif //WEBSERVER_HTTP_CONN_H
