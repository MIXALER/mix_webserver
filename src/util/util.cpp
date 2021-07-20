//
// Created by yuanh on 2021/4/19.
//

#include "util.h"

void Util::Init(int timeslot)
{
    timeslot_ = timeslot;
}

int Util::SetNoBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}


void Util::AddFd(int epoll_fd, int fd, bool one_shot, int trig_mode)
{
    epoll_event event;
    event.data.fd = fd;

    if (trig_mode == 1)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    SetNoBlocking(fd);
}

void Util::SigHandler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(pipe_fd_[1], (char *) &msg, 1, 0);
    errno = save_errno;
}

void Util::AddSig(int sig, void (*handler)(int), bool restart)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

void Util::TimerHandler()
{
    timer_list_.Tick();
    alarm(timeslot_);
}

void Util::ShowError(int conn_fd, const char *info)
{
    send(conn_fd, info, strlen(info), 0);
    close(conn_fd);
}

int *Util::pipe_fd_ = 0;
int Util::epoll_fd_ = 0;



