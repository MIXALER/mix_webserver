//
// Created by yuanh on 2021/4/3.
//

#ifndef WEBSERVER_LOG_H
#define WEBSERVER_LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include <memory.h>
#include "../block_queue/block_queue.h"

using namespace std;

class Log
{
// c++ 11 以后，使用局部变量懒汉不用加锁
public:
    static Log *GetInstance()
    {
        static Log instance;
        return &instance;
    }

    static void *FlushLogThread(void *args)
    {
        Log::GetInstance()->AsyncWriteLog();
        return nullptr;
    }

    bool Init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000,
              int max_queue_size = 0);

    void WriteLog(int level, const char *format, ...);

    void Flush();

private:
    Log();

    virtual ~Log();

    void *AsyncWriteLog()
    {
        string single_log;
        // 从阻塞队列中取出一个日志 string，写入文件
        while (log_queue_->Pop(single_log))
        {
            mutex_.Lock();
            fputs(single_log.c_str(), fp_);
            mutex_.Unlock();
        }
        return nullptr;
    }

private:
    char dir_name_[128]; // 路径名
    char log_name_[128];
    int split_lines_;
    int log_buf_size_;
    long long count_; // 行数记录
    int today_;
    FILE *fp_; // 打开 log 的文件指针
    char *buff_;
    BlockQueue<string> *log_queue_; // 阻塞队列
    bool is_async_; // 是否同步标志位
    Locker mutex_;
    int close_log_;
};


#define LOG_DEBUG(format, ...) if(close_log_ == 0){Log::GetInstance()->WriteLog(0, format, ##__VA_ARGS__); Log::GetInstance()->Flush();}
#define LOG_INFO(format, ...) if(close_log_ == 0){Log::GetInstance()->WriteLog(1, format, ##__VA_ARGS__); Log::GetInstance()->Flush();}
#define LOG_WARN(format, ...) if(close_log_ == 0){Log::GetInstance()->WriteLog(2, format, ##__VA_ARGS__); Log::GetInstance()->Flush();}
#define LOG_ERROR(format, ...) if(close_log_ == 0){Log::GetInstance()->WriteLog(3, format, ##__VA_ARGS__); Log::GetInstance()->Flush();}


#endif //WEBSERVER_LOG_H
