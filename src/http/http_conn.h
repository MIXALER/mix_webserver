//
// Created by yuanh on 2021/4/3.
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
#include <fstream>

#include "../lock/locker.h"
#include "../mysql/sql_conn_pool.h"
#include "../timer/list_timer.h"
#include "../log/log.h"


class HttpConn
{
public:
    static const int kFileNameLen = 200;
    static const int kReadBufferSize = 2048;
    static const int kWriteBufferSize = 1024;
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

    void CloseConn(bool real_close = true);

    void Process();

    bool ReadOnce();

    bool Write();

    sockaddr_in *GetAddress()
    {
        return &address_;
    }

    void InitMysqlResult(ConnectionPool *conn_pool);

    int timer_flag_;

    int improve_;
private:
    void Init();

    HttpCode ProcessRead();

    bool ProcessWrite(HttpCode ret);

    HttpCode ParseRequestLine(char *text);

    HttpCode ParseHeaders(char *text);

    HttpCode ParseContent(char *text);

    HttpCode DoRequest();

    char *GetLine()
    {
        return read_buf_ + start_line_;
    }

    LineStatus ParseLine();

    void UnMap();

    bool AddResponse(const char *format, ...);

    bool AddContent(const char *content);

    bool AddStatusLine(int status, const char *title);

    bool AddHeaders(int content_len);

    bool AddContentType();

    bool AddContentLen(int content_len);

    bool AddLinger();

    bool AddBlankLine();


public:
    static int epoll_fd_;
    static int user_count_;
    MYSQL *mysql_;
    int state_; // 读为 0，写为 1

private:
    int sock_fd_;
    sockaddr_in address_;
    char read_buf_[kReadBufferSize];
    int read_idx_;
    int checked_idx_;
    int start_line_;
    char write_buf_[kWriteBufferSize];
    int write_idx_;
    CheckState check_state_;
    Method method_;
    char real_file_[kFileNameLen];
    char *url_;
    char *version_;
    char *host_;
    int content_len_;
    bool linger_;
    char *file_address_;
    struct stat file_state_;
    struct iovec iv_[2];
    int iv_count_;
    int cgi_; // 是否启用的 POST
    char *string_; // 存储请求头数据
    int bytes_to_send_;
    int bytes_have_send_;
    char *doc_root_;

    map<string, string> users_;
    int trig_mode_;
    int close_log_;

    char sql_user_[100];
    char sql_passwd_[100];
    char sql_name_[100];

};

#endif //WEBSERVER_HTTP_CONN_H
