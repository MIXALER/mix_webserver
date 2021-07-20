//
// Created by yuanh on 2021/7/2.
//

#include "webserver.h"

//void WebServer::init(uint16_t port)
//{
//    = port;
//}


void WebServer::LogWrite()
{
    if (close_log_ == 0)
    {
        cout << "初始化日志" << endl;
        if (log_write_ == 1)
        {
            cout << "日志设定为异步模式" << endl;
            Log::GetInstance()->Init("./ServerLog", close_log_, 2000, 800000, 800);
        } else
        {
            cout << "sys:日志功能关闭" << endl;
            Log::GetInstance()->Init("./ServerLog", close_log_, 2000, 800000, 0);
        }
    }
}

void WebServer::Init(uint16_t port, string user, string passwd, string database_name, int log_write, int opt_linger,
                     int trig_mode, int sql_num, int thread_num, int close_log, int actor_mode)
{
    port_ = port;
    user_ = user;
    passwd_ = passwd;
    database_name_ = database_name;
    log_write_ = log_write;
    sql_num_ = sql_num;
    thread_num_ = thread_num;
    opt_linger_ = opt_linger;
    trig_mode_ = trig_mode;
    close_log_ = close_log;
    actor_mode_ = actor_mode;
}

WebServer::WebServer()
{
    users_ = new HttpConn[kMaxFd];

    char server_path[200];
    getcwd(server_path, 200);
    char root[6] = "/html";
    root_ = (char *) malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(root_, server_path);
    strcat(root_, root);

    user_timer_ = new ClientData[kMaxFd];
}

WebServer::~WebServer()
{
    close(epoll_fd_);
    close(listen_fd_);
    close(pipe_fd_[1]);
    close(pipe_fd_[0]);
    delete[] users_;
    delete[] user_timer_;
    delete pool_;
}

void WebServer::ThreadPool()
{
    pool_ = new ThreadP<HttpConn>(actor_mode_, conn_pool_, thread_num_);
}

void WebServer::SqlPool()
{
    conn_pool_ = ConnectionPool::GetInstance();
    conn_pool_->Init("localhost", user_, passwd_, database_name_, 3306, sql_num_, close_log_);

    users_->InitMysqlResult(conn_pool_);
}

void WebServer::TrigMode()
{
    if (trig_mode_ == 0)
    {
        listen_trig_mode_ = 0;
        conn_trig_mode_ = 0;
    } else if (trig_mode_ == 1)
    {
        listen_trig_mode_ = 0;
        conn_trig_mode_ = 1;
    } else if (trig_mode_ == 2)
    {
        listen_trig_mode_ = 1;
        conn_trig_mode_ = 0;
    } else if (trig_mode_ == 3)
    {
        listen_trig_mode_ = 1;
        conn_trig_mode_ = 1;
    }
}

void WebServer::EventListen()
{
    // 网络编程基础步骤
    listen_fd_ = socket(PF_INET, SOCK_STREAM, 0);
    assert(listen_fd_ >= 0);

    if (opt_linger_ == 0)
    {
        struct linger tmp = {0, 1};
        setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    } else if (opt_linger_ == 1)
    {
        struct linger tmp = {1, 1};
        setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port_);

    int flag = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(listen_fd_, (struct sockaddr *) &address, sizeof(address));
    assert(ret >= 0);
    ret = listen(listen_fd_, 5);
    assert(ret >= 0);

    utils_.Init(kTimeSlot);

    epoll_event events[kMaxEventNumber];
    epoll_fd_ = epoll_create(5);
    assert(epoll_fd_ != -1);

    utils_.AddFd(epoll_fd_, listen_fd_, false, listen_trig_mode_);
    HttpConn::epoll_fd_ = epoll_fd_;

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipe_fd_);
    assert(ret != -1);
    utils_.SetNoBlocking(pipe_fd_[1]);
    utils_.AddFd(epoll_fd_, pipe_fd_[0], false, 0);

    utils_.AddSig(SIGPIPE, SIG_IGN);
    utils_.AddSig(SIGALRM, utils_.SigHanler, false);
    utils_.AddSig(SIGTERM, utils_.SigHandler, false);

    alarm(kTimeSlot);

    Util::pipe_fd_ = pipe_fd_;
    Util::epoll_fd_ = epoll_fd_;
}

void WebServer::Timer(int conn_fd, struct sockaddr_in client_addr)
{
    users_[conn_fd].Init(conn_fd, client_addr, root_, conn_trig_mode_, close_log_, user_, passwd_, database_name_);

    user_timer_[conn_fd].address_ = client_addr;
    user_timer_[conn_fd].sock_fd_ = conn_fd;
    UtilTimer *timer = new UtilTimer;
    timer->user_data_ = &user_timer_[conn_fd];
    timer->cb_func_ = CbFunc;
    time_t cur = time(nullptr);
    timer->expire_ = cur + 3 * kTimeSlot;
    user_timer_[conn_fd].timer_ = timer;
    utils_.timer_list_.AddTimer(timer);
}

void WebServer::AdjustTimer(UtilTimer *timer)
{
    time_t cur = time(nullptr);
    timer->expire_ = cur + 3 * kTimeSlot;
    utils_.timer_list_.AdjustTimer(timer);

    LOG_INFO("%s", "adjust timer once");
}

void WebServer::DealTimer(UtilTimer *timer, int sock_fd)
{
    timer->cb_func_(&user_timer_[sock_fd]);
    if (timer != nullptr)
    {
        utils_.timer_list.DelTimer(timer);
    }

    LOG_INFO("close fd %d", user_timer_[sock_fd].sock_fd_);
}

bool WebServer::DealClientData()
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    if (listen_trig_mode_ == 0)
    {
        int conn_fd = accept(listen_fd_, (struct sockaddr *) &client_addr, &client_addr_len);
        if (conn_fd < 0)
        {
            LOG_ERROR("%s:errno is:%d", "accept error", errno)
            return false;
        }
        if (HttpConn::user_count_ >= kMaxFd)
        {
            utils_.ShowError(conn_fd, "Internal server busy");
            LOG_ERROR("%s", "Interval server busy");
            return false;
        }
        Timer(conn_fd, client_addr);
    } else
    {
        while (1)
        {
            int conn_fd = accept(listen_fd_, (struct sockaddr *) &client_addr, &client_addr_len);
            if (conn_fd < 0)
            {
                LOG_ERROR("%s:errno is:%d", "accept error", errno);
                break;
            }
            if (HttpConn::user_count_ >= kMaxFd)
            {
                utils_.ShowError(conn_fd, "Internal server busy");
                LOG_ERROR("%s", "Internal server busy");
                break;
            }
            Timer(conn_fd, client_addr);
        }
        return false;
    }
    return true;
}

bool WebServer::DealWithSignal(bool &timeout, bool &stop_server)
{
    int ret = 0;
    int sig;
    char signals[1024];
    ret = recv(pipe_fd_[0], signals, sizeof(signals), 0);
    if (ret == -1)
    {
        return false;
    } else if (ret == 0)
    {
        return false;
    } else
    {
        for (int i = 0; i < ret; ++i)
        {
            switch (signals[i])
            {
                case SIGALRM:
                {
                    timeout = true;
                    break;
                }
                case SIGTERM:
                {
                    stop_server = true;
                    break;
                }
            }
        }
    }
    return true;
}

void WebServer::DealWithThread(int sock_fd)
{
    UtilTimer *timer = user_timer_[sock_fd].timer_;

    if (actor_mode_ == 1)
    {
        if (timer)
        {
            AdjustTimer(timer);
        }
        pool_->Append(users_ + sock_fd, 0);

        while (true)
        {
            if (users_[sock_fd].improve_ == 1)
            {
                DealTimer(timer, sock_fd);
                users_[sock_fd].timer_flag_ = 0;
            }
            users_[sock_fd].improve_ = 0;
            break;
        }
    } else
    {
        if (users_[sock_fd].ReadOnce())
        {
            LOG_INFO("deal with the client(%s)", inet_ntoa(users_[sock_fd].GetAddress()->sin_addr));

            pool_->AppendP(users_ + sock_fd);
            if (timer)
            {
                AdjustTimer(timer);
            }
        } else
        {
            DealTimer(timer, sock_fd);
        }
    }
}

void WebServer::DealWithWrite(int sock_fd)
{
    UtilTimer *timer = user_timer_[sock_fd].timer_;
    if (actor_mode_ == 1)
    {
        if (timer)
        {
            AdjustTimer(timer);
        }
        pool_->Append(users_ + sock_fd, 1);
        while (true)
        {
            if (users_[sock_fd].improve_ == 1)
            {
                DealTimer(timer, sock_fd);
                users_[sock_fd].timer_flag_ = 0;
            }
            users_[sock_fd].improve_ = 0;
            break;
        }
    } else
    {
        if (users_[sock_fd].Write())
        {
            LOG_INFO("send data to the client(%s)", inet_ntoa(users_[sock_fd].GetAddress()->sin_addr));
            if (timer)
            {
                AdjustTimer(timer);
            }
        } else
        {
            DealTimer(timer, sock_fd);
        }
    }
}

void WebServer::EventLoop()
{
    bool timeout = false;
    bool stop_server = false;

    while (!stop_server)
    {
        int number = epoll_wait(epoll_fd_, events_, kMaxEventNumber, -1);
        if (number < 0 && errno != EINTR)
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (int i = 0; i < number; ++i)
        {
            int sock_fd = events_[i].data.fd;

            if (sock_fd == listen_fd_)
            {
                bool flag = DealClientData();
                if (flag == false)
                    continue;
            } else if (events_[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                UtilTimer *timer = user_timer_[sock_fd].timer_;
                DealTimer(timer, sock_fd);
            } else if ((sock_fd == pipe_fd_[0]) && (events_[i].events && EPOLLIN))
            {
                bool flag = DealWithSignal(timeout, stop_server);
                if (flag == false)
                    LOG_ERROR("%s", "deal with client data failure");
            } else if (events_[i].events & EPOLLIN)
            {
                DealWithThread(sock_fd);
            } else if (events_[i].events & EPOLLOUT)
            {
                DealWithWrite(sock_fd);
            }
        }
        if (timeout)
        {
            utils_.TimeHandler();

            LOG_INFO("%s", "timer tick");

            timeout = false;
        }
    }
}
