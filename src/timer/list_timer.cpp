//
// Created by yuanh on 2021/4/19.
//

#include "list_timer.h"


SortTimerList::SortTimerList()
{
    head_ = nullptr;
    tail_ = nullptr;
}

SortTimerList::~SortTimerList()
{
    UtilTimer *tmp = head_;
    while (tmp != nullptr)
    {
        head_ = tmp->next_;
        delete tmp;
        tmp = head_;
    }
}

void SortTimerList::AddTimer(UtilTimer *timer)
{
    if (timer == nullptr)
    {
        return;
    }
    if (head_ == nullptr)
    {
        head_ = tail_ = timer;
        return;
    }
    if (timer->expire_ < head_->expire_)
    {
        timer->next_ = head_;
        head_->prev_ = timer;
        head_ = timer;
        return;
    }
    AddTimer(timer, head_);
}

void SortTimerList::AdjustTimer(UtilTimer *timer)
{
    if (timer == nullptr)
    {
        return;
    }
    UtilTimer *tmp = timer->next_;
    if (tmp == nullptr || (timer->expire_ < tmp->expire_))
    {
        return;
    }
    if (timer == head_)
    {
        head_ = head_->next_;
        head_->prev_ = nullptr;
        timer->next_ = nullptr;
        AddTimer(timer, head_);
    } else
    {
        timer->prev_->next_ = timer->next_;
        timer->next_->prev_ = timer->prev_;
        AddTimer(timer, timer->next_);
    }
}

void SortTimerList::DelTimer(UtilTimer *timer)
{
    if (timer == nullptr)
    {
        return;
    }
    if ((timer == head_) && (timer == tail_))
    {
        delete timer;
        head_ = nullptr;
        tail_ = nullptr;
        return;
    }
    if (timer == head_)
    {
        head_ = head_->next_;
        head_->prev_ = nullptr;
        delete timer;
        return;
    }
    if (timer == tail_)
    {
        tail_ = tail_->prev_;
        tail_->next_ = nullptr;
        delete timer;
        return;
    }
    timer->prev_->next_ = timer->next_;
    timer->next_->prev_ = timer->prev_;
    delete timer;
}

void SortTimerList::Tick()
{
    if (head_ == nullptr)
    {
        return;
    }
    time_t cur = time(nullptr);
    UtilTimer *tmp = head_;
    while (tmp != nullptr)
    {
        if (cur < tmp->expire_)
        {
            break;
        }
        tmp->cb_func_(tmp->user_data_);
        head_ = tmp->next_;
        if (head_)
        {
            head_->prev_ = nullptr;
        }
        delete tmp;
        tmp = head_;
    }
}

void SortTimerList::AddTimer(UtilTimer *timer, UtilTimer *list_head)
{
    UtilTimer *prev = list_head;
    UtilTimer *tmp = prev->next_;
    while (tmp != nullptr)
    {
        if (timer->expire_ < tmp->expire_)
        {
            prev->next_ = timer;
            timer->next_ = tmp;
            tmp->prev_ = timer;
            timer->prev_ = prev;
            break;
        }
        prev = tmp;
        tmp = tmp->next_;
    }
    if (tmp == nullptr)
    {
        prev->next_ = timer;
        timer->prev_ = prev;
        timer->next_ = nullptr;
        tail_ = timer;
    }
}

void CbFunc(ClientData *user_data)
{
    epoll_ctl(Util::epoll_fd_, EPOLL_CTL_DEL, user_data->sock_fd_, 0);
    assert(user_data);
    close(user_data->sock_fd_);
    HttpConn::user_count_--;
}
