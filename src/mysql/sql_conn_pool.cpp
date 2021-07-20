//
// Created by yuanh on 2021/4/11.
//

#include "sql_conn_pool.h"

ConnectionPool::ConnectionPool()
{
    cur_conn_ = 0;
    free_conn_ = 0;
}

ConnectionPool::~ConnectionPool()
{
    DestroyPool();
}

MYSQL *ConnectionPool::GetConnection()
{
    MYSQL *conn = nullptr;
    if (conn_list_.size() == 0)
        return nullptr;
    reserve_.Wait();
    lock_.Lock();

    conn = conn_list_.front();
    conn_list_.pop_front();
    free_conn_--;
    cur_conn_++;

    lock_.Unlock();
    return conn;
}

bool ConnectionPool::ReleaseConnection(MYSQL *conn)
{
    if (conn == nullptr)
        return false;
    lock_.Lock();
    conn_list_.push_back(conn);
    free_conn_++;
    cur_conn_--;
    lock_.Lock();
    reserve_.Post();
    return true;
}

// 当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
int ConnectionPool::GetFreeConn()
{
    return free_conn_;
}


ConnectionPool *ConnectionPool::GetInstance()
{
    static ConnectionPool conn_pool;
    return &conn_pool;
}

void ConnectionPool::Init(string url, string user, string passwd, string database_name, uint16_t port, int max_conn,
                          int close_log)
{
    url_ = url;
    port_ = port;
    user_ = user;
    passwd_ = passwd;
    database_name_ = database_name;
    close_log_ = close_log;
    for (int i = 0; i < max_conn; ++i)
    {
        MYSQL *conn = nullptr;
        conn = mysql_init(conn);

        if (conn == nullptr)
        {
            LOG_ERROR("MYSQL ERROR");
            exit(1);
        }
        conn = mysql_real_connect(conn, url_.c_str(), user_.c_str(), passwd_.c_str(),
                                  database_name_.c_str(), port_, nullptr, 0);
        if (conn == nullptr)
        {
            LOG_ERROR("MYSQL ERROR");
            exit(1);
        }
        conn_list_.push_back(conn);
        free_conn_++;
    }

    reserve_ = Sem(free_conn_);

    max_conn_ = free_conn_;

}

void ConnectionPool::DestroyPool()
{
    lock_.Lock();
    if (conn_list_.size() > 0)
    {
        list<MYSQL *>::iterator it;
        for (it = conn_list_.begin(); it != conn_list_.end(); ++it)
        {
            MYSQL *conn = *it;
            mysql_close(conn);
        }
        cur_conn_ = 0;
        free_conn_ = 0;
        conn_list_.clear();
    }
    lock_.Unlock();

}

ConnectionRAII::ConnectionRAII(MYSQL **conn, ConnectionPool *conn_pool)
{
    *conn = conn_pool->GetConnection();

    conRAII_ = *conn;
    poolRAII_ = conn_pool;

}

ConnectionRAII::~ConnectionRAII()
{
    poolRAII_->ReleaseConnection(conRAII_);
}
