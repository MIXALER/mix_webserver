//
// Created by yuanh on 2021/4/3.
//

#include "http_conn.h"

// 定义 http 响应信息
const char *kOk200Title = "OK";
const char *kError400Title = "Bad Request";
const char *kError400Form = "Your Request has bad syntax or is inherently to satisfy.\n";
const char *kError403Title = "Forbidden";
const char *kError403Form = "You do not have permission to get file from this server.\n";
const char *kError404Title = "Not Found";
const char *kError404Form = "The requested file was not found on this server.\n";
const char *kError500Title = "Internal Error";
const char *kError500Form = "There was an unusual problem serving the request file.\n";

Locker kLock;
map<string, string> kUsers;

int SetNonBlocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// 将内核事件表注册读事件，ET 模式，选择开启 EPOLLONESHOT
void AddFd(int epoll_fd, int fd, bool one_shot, int trig_mode)
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
    SetNonBlocking(fd);
}

void HttpConn::Init(int sock_fd, const sockaddr_in &addr, char *root, int trig_mode, int close_log, string user,
                    string passwd, string sql_name)
{
    sock_fd_ = sock_fd;
    address_ = addr;
    user_count_++;
    doc_root_ = root;
    trig_mode_ = trig_mode;
    AddFd(epoll_fd_, sock_fd, true, trig_mode_);
    strcpy(sql_user_, user.c_str());
    strcpy(sql_passwd_, passwd.c_str());
    strcpy(sql_name_, sql_name.c_str());

    Init();
}

void HttpConn::Init()
{
    mysql_ = NULL;
    bytes_to_send_ = 0;
    bytes_have_send_ = 0;
    check_state_ = kCheckStateRequestLine;
    linger_ = false;
    method_ = kGet;
    url_ = 0;
    version_ = 0;
    content_len_ = 0;
    host_ = 0;
    start_line_ = 0;
    checked_idx_ = 0;
    read_idx_ = 0;
    write_idx_ = 0;
    cgi_ = 0;
    state_ = 0;
    timer_flag_ = 0;

    memset(read_buf_, '\0', kReadBufferSize);
    memset(write_buf_, '\0', kWriteBufferSize);
    memset(real_file_, '\0', kFileNameLen);
}

// 从内核事件表中删除描述符
void RemoveFd(int epoll_fd, int fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void HttpConn::CloseConn(bool real_close)
{
    if (real_close && (sock_fd_ != -1))
    {
        printf("close %d\n", sock_fd_);
        RemoveFd(epoll_fd_, sock_fd_);
        sock_fd_ = -1;
        user_count_--;
    }
}

int HttpConn::user_count_ = 0;
int HttpConn::epoll_fd_ = -1;


void HttpConn::Process()
{

    HttpCode read_ret = ProcessRead();
}

bool HttpConn::ReadOnce()
{
    return false;
}

bool HttpConn::Write()
{
    return false;
}

void HttpConn::InitMysqlResult(ConnectionPool *conn_pool)
{

}

HttpConn::HttpCode HttpConn::ProcessRead()
{
    LineStatus line_status = kLineOk;
    HttpCode ret = kNoRequest;
    char *text = 0;
    while ((check_state_ == kCheckStateContent && line_status == kLineOk) || ((line_status = ParseLine()) == kLineOk))
    {
        text = GetLine();
        start_line_ = checked_idx_;
        LOG_INFO("%s", text);
        switch (check_state_)
        {
            case kCheckStateRequestLine:
            {
                ret = ParseRequestLine(text);
                if (ret == kBadRequest)
                    return kBadRequest;
                break;
            }
            case kCheckStateHeader:
            {
                ret = ParseHeaders(text);
                if (ret == kBadRequest)
                    return kBadRequest;
                else if (ret == kGetRequest)
                {
                    return DoRequest();
                }
            }
            default:
                return kInternalError;
        }

    }
    return HttpConn::kClosedConnection;
}

bool HttpConn::ProcessWrite(HttpConn::HttpCode ret)
{
    return false;
}

// 解析 http 请求行，获得请求方法，目标 url 及版本号
HttpConn::HttpCode HttpConn::ParseRequestLine(char *text)
{
    url_ = strpbrk(text, "\t");
    if (!url_)
    {
        return kBadRequest;
    }
    *url_++ = '\0';
    char *method = text;
    if (strcasecmp(method, "GET") == 0)
        method_ = kGet;
    else if (strcasecmp(method, "POST") == 0)
    {
        method_ = kPost;
        cgi_ = 1;
    } else
        return kBadRequest;
    url_ += strspn(url_, "\t");
    version_ = strpbrk(url_, "\t");
    if (!version_)
        return kBadRequest;
    *version_++ = '\0';
    version_ += strspn(version_, "\t");
    if (strcasecmp(version_, "HTTP/1.1") != 0)
        return kBadRequest;
    if (strncasecmp(url_, "http://", 7) == 0)
    {
        url_ += 7;
        url_ = strchr(url_, '/');
    }

    if (strncasecmp(url_, "https://", 8) == 0)
    {
        url_ += 8;
        url_ = strchr(url_, '/');
    }

    if (!url_ || url_[0] != '/')
        return kBadRequest;
    // 当 url_ 为 / 时，显示判断界面
    if (strlen(url_) == 1)
        strcat(url_, "judge.html");
    check_state_ = kCheckStateHeader;
    return kNoRequest;
}

// 解析 http 请求的一个头部信息
HttpConn::HttpCode HttpConn::ParseHeaders(char *text)
{
    if (text[0] == '\0')
    {
        if (content_len_ != 0)
        {
            check_state_ = kCheckStateContent;
            return kNoRequest;
        }
        return kGetRequest;
    } else if (strncasecmp(text, "Connection:", 11) == 0)
    {
        text += 11;
        text += strspn(text, "\t");
        if (strcasecmp(text, "keep-alive") == 0)
        {
            linger_ = true;
        }
    } else if (strncasecmp(text, "Content-length:", 15) == 0)
    {
        text += 15;
        text += strspn(text, " \t");
        content_len_ = atol(text);
    } else if (strncasecmp(text, "Host:", 5) == 0)
    {
        text += 5;
        text += strspn(text, " \t");
        content_len_ = atol(text);
    } else
    {
        LOG_INFO("oop!unknown header: %s", text);
    }
    return kNoRequest;
}

// 判断 http 请求是否被完整读入
HttpConn::HttpCode HttpConn::ParseContent(char *text)
{
    if (read_idx_ >= (content_len_ + checked_idx_))
    {
        text[content_len_] = '\0';
        // POST 请求最后为输入的用户名和密码
        string_ = text;
        return kGetRequest;
    }
    return kNoRequest;
}

HttpConn::HttpCode HttpConn::DoRequest()
{
    strcpy(real_file_, doc_root_);
    int len = strlen(doc_root_);
    const char *p = strrchr(url_, '/');

    if (cgi_ == 1 && (*(p + 1) == '2' || *(p + 1) == '3'))
    {
        // 根据标志判断是登录检测还是注册检测
        char flag = url_[1];
        char *url_real = (char *) malloc(sizeof(char) * 200);
        strcpy(url_real, "/");
        strcat(url_real, url_ + 2);
        strncpy(real_file_ + len, url_real, kFileNameLen - len - 1);
        free(url_real);

        // 将用户名和密码提出来
        char name[100], passwd[100];
        int i;
        for (i = 5; string_[i] != '&'; i++)
        {
            name[i - 5] = string_[i];
        }
        name[i - 5] = '\0';

        int j = 0;

    }
    return kGetRequest;
}

HttpConn::LineStatus HttpConn::ParseLine()
{
    char tmp;
    for (; checked_idx_ < read_idx_; checked_idx_++)
    {

    }
    return HttpConn::kLineBad;
}

void HttpConn::UnMap()
{

}

bool HttpConn::AddResponse(const char *format, ...)
{
    return false;
}

bool HttpConn::AddContent(const char *content)
{
    return false;
}

bool HttpConn::AddStatusLine(int status, const char *title)
{
    return false;
}

bool HttpConn::AddHeaders(int content_len)
{
    return false;
}

bool HttpConn::AddContentType()
{
    return false;
}

bool HttpConn::AddContentLen(int content_len)
{
    return false;
}

bool HttpConn::AddLinger()
{
    return false;
}

bool HttpConn::AddBlankLine()
{
    return false;
}



