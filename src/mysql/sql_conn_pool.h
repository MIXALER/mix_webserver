//
// Created by yuanh on 2021/4/3.
//

#ifndef WEBSERVER_SQL_CONN_POOL_H
#define WEBSERVER_SQL_CONN_POOL_H

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include "../lock/locker.h"
#include "../log/log.h"

using namespace std;

class ConnectionPool
{
public:
    MYSQL *GetConnection();

    bool ReleaseConnection(MYSQL *conn);

    // 获取空闲连接数
    int GetFreeConn();

    void DestroyPool();

    // 单例模式
    static ConnectionPool *GetInstance();

    void Init(string url, string user, string passwd, string database_name, uint16_t port, int max_conn, int close_log);

private:
    ConnectionPool();

    ~ConnectionPool();

    int max_conn_;
    int cur_conn_;
    int free_conn_;
    Locker lock_;
    list<MYSQL *> conn_list_;
    Sem reserve_;

public:
    string url_;
    uint16_t port_;
    string user_;
    string passwd_;
    string database_name_;
    int close_log_;
};

class ConnectionRAII
{
public:
    ConnectionRAII(MYSQL **conn, ConnectionPool *conn_pool);

    ~ConnectionRAII();

private:
    MYSQL *conRAII_;
    ConnectionPool *poolRAII_;
};

#endif //WEBSERVER_SQL_CONN_POOL_H
