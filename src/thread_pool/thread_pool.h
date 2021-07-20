//
// Created by yuanh on 2021/4/3.
//

#ifndef WEBSERVER_THREAD_POOL_H
#define WEBSERVER_THREAD_POOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>

#include "../lock/locker.h"
#include "../mysql/sql_conn_pool.h"


template<typename T>
class ThreadP
{
public:
    ThreadP(int actor_mode, ConnectionPool *conn_pool, int thread_num = 8, int max_request = 10000);

    ~ThreadP();

    // 往请求队列中添加任务
    bool Append(T *request, int state);

    bool AppendP(T *request);

private:
    // 工作线程运行的函数，它不断从工作队列中取出任务并执行
    static void *Worker(void *arg);

    void Run();

private:
    int thread_num_;            // 线程数
    int max_requests_;          // 请求队列中的最大请求数
    pthread_t *threads_;        // 描述线程池的数组
    list<T *> work_queue_;      // 请求队列
    Locker queue_locker_;       // 保护请求队列的互斥锁
    Sem queue_state_;           // 是否有任务需要处理
    ConnectionPool *conn_pool_; // 数据库
    int actor_mode_;            // 模型切换
};

template<typename T>
ThreadP<T>::ThreadP(int actor_mode, ConnectionPool *conn_pool, int thread_num, int max_request)
        :actor_mode_(actor_mode), thread_num_(thread_num), max_requests_(max_request), threads_(NULL),
         conn_pool_(conn_pool)
{
    if (thread_num <= 0 || max_request <= 0)
        throw std::exception();
    threads_ = new pthread_t[thread_num_];
    if (!threads_)
    {
        throw std::exception();
    }
    // 创建 thread_num_ 个线程，并将它们都设置为脱离线程
    for (int i = 0; i < thread_num; ++i)
    {
        if (pthread_create(threads_ + i, NULL, Worker, this) != 0)
        {
            delete[] threads_;
            throw std::exception();
        }
        // 取代 thread_join
        if (pthread_detach(threads_[i]))
        {
            delete[] threads_;
            throw std::exception();
        }
    }
}

template<typename T>
ThreadP<T>::~ThreadP()
{
    delete[] threads_;
}

template<typename T>
bool ThreadP<T>::Append(T *request, int state)
{
    queue_locker_.Lock();
    if (work_queue_.size() >= max_requests_)
    {
        queue_locker_.Unlock();
        return false;
    }
    request->state_ = state;
    work_queue_.push_back(request);
    queue_locker_.Unlock();
    queue_state_.Post();
    return true;
}

template<typename T>
bool ThreadP<T>::AppendP(T *request)
{
    queue_locker_.Lock();
    if (work_queue_.size() >= max_requests_)
    {
        queue_locker_.Unlock();
        return false;
    }
    work_queue_.push_back(request);
    queue_locker_.Unlock();
    queue_state_.Post();
    return true;
}

template<typename T>
void *ThreadP<T>::Worker(void *arg)
{
    ThreadP *pool = (ThreadP *) arg;
    pool->Run();
    return nullptr;
}

template<typename T>
void ThreadP<T>::Run()
{
    while (true)
    {
        queue_state_.Wait();
        queue_locker_.Lock();
        if (work_queue_.empty())
        {
            queue_locker_.Unlock();
            continue;
        }
        T *request = work_queue_.front();
        work_queue_.pop_front();
        queue_locker_.Unlock();
        if (!request)
            continue;
        if (actor_mode_ == 1)
        {
            if (request->state_ == 0)
            {
                if (request->read_once_())
                {
                    request->improve_ = 1;
                    ConnectionRAII mysql_conn(&request->mysql_, conn_pool_);
                    request->Process();
                } else
                {
                    request->improve_ = 1;
                    request->timer_flag_ = 1;
                }
            } else
            {
                if (request->Write())
                {
                    request->improve_ = 1;
                } else
                {
                    request->improve_ = 1;
                    request->timer_flag_ = 1;
                }
            }
        } else
        {
            ConnectionRAII mysql_conn(&request->mysql_, conn_pool_);
            request->Process();
        }
    }
}

#endif //WEBSERVER_THREAD_POOL_H
