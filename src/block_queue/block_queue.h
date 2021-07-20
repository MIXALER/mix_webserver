//
// Created by yuanh on 2021/4/3.
//

#ifndef WEBSERVER_BLOCK_QUEUE_H
#define WEBSERVER_BLOCK_QUEUE_H

#include <iostream>
#include <pthread.h>

#include <sys/time.h>
#include "../lock/locker.h"

using namespace std;

template<typename T>
class BlockQueue
{
public:
    BlockQueue(int max_size = 1000)
    {
        if (max_size <= 0)
        {
            exit(-1);
        }
        max_size_ = max_size;
        array_ = new T[max_size_];
        size_ = 0;
        front_ = -1;
        back_ = -1;
    }

    ~BlockQueue()
    {
        mutex_.Lock();
        if (array_ != NULL)
        {
            delete[] array_;
        }
        mutex_.Unlock();
    }

    bool Full()
    {
        mutex_.Lock();
        if (size_ >= max_size_)
        {
            mutex_.Unlock();
            return true;
        }
        mutex_.Unlock();
        return false;
    }

    void Clear()
    {
        mutex_.Lock();
        size_ = 0;
        front_ = -1;
        back_ = -1;
        mutex_.Unlock();
    }

    bool Empty()
    {
        mutex_.Lock();
        if (size_ == 0)
        {
            mutex_.Unlock();
            return true;
        }
        mutex_.Unlock();
        return false;
    }

    bool Front(T &value)
    {
        mutex_.Lock();
        if (size_ == 0)
        {
            mutex_.Unlock();
            return false;
        }
        value = array_[front_];
        mutex_.Unlock();
        return true;
    }

    bool Back(T &value)
    {
        mutex_.Lock();
        if (size_ == 0)
        {
            mutex_.Unlock();
            return false;
        }
        value = array_[back_];
        mutex_.Unlock();
        return true;
    }

    int Size()
    {
        int res = 0;
        mutex_.Lock();
        res = size_;
        mutex_.Unlock();
        return res;
    }

    int MaxSize()
    {
        int res = 0;
        mutex_.Lock();
        res = max_size_;
        mutex_.Unlock();
        return res;
    }

    bool Push(const T &item)
    {
        mutex_.Lock();
        if (size_ >= max_size_)
        {
            cond_.Broadcast();
            mutex_.Unlock();
            return false;
        }
        back_ = (back_ + 1) % max_size_;
        array_[back_] = item;
        size_++;
        cond_.Broadcast();
        mutex_.Unlock();
        return true;
    }

    bool Pop(T &item)
    {
        mutex_.Lock();
        while (size_ <= 0)
        {
            if (cond_.Wait(mutex_.Get()))
            {
                mutex_.Unlock();
                return false;
            }
        }
        front_ = (front_ + 1) % max_size_;
        item = array_[front_];
        size_--;
        mutex_.Unlock();
        return true;
    }

private:
    Locker mutex_;
    Cond cond_;
    T *array_;
    int size_;
    int max_size_;
    int front_;
    int back_;
};

#endif //WEBSERVER_BLOCK_QUEUE_H
