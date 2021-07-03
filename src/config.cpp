//
// Created by yuanh on 2021/7/2.
//

#include "config.h"

void Config::ParseConfig(int argc, char **argv)
{
    int opt;
    const char *str = "p:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
            case 'p':
            {
                port_ = atoi(optarg);
                break;
            }
            case 'l':
            {
                log_write_ = atoi(optarg);
            }
            case 'm':
            {
                trig_mode_ = atoi(optarg);
            }
            case '0':
            {
                opt_linger_ = atoi(optarg);
            }
            case 's':
            {
                sql_num_ = atoi(optarg);
            }
            case 't':
            {
                thread_num_ = atoi(optarg);
            }
            case 'c':
            {
                close_log_ = atoi(optarg);
            }
            case 'a':
            {
                actor_mode_ = atoi(optarg);
            }


            default:
                break;
        }
    }
}

Config::Config()
{
    // 默认端口号
    port_ = 23335;
    // 日志写入方式，默认同步
    log_write_ = 0;
    // 触发组合模式，默认 listenfd LT + connfd LT
    trig_mode_ = 0;
    // listenfd 触发模式 默认 LT
    listen_fd_trig_mode_ = 0;
    //connfd 触发模式 LT
    conn_trig_mode_ = 0;
    // 优雅关闭链接，默认不使用
    opt_linger_ = 0;
    // 数据库连接池数量，默认 8
    sql_num_ = 8;
    //线程池线程数量，默认 8
    thread_num_ = 8;
    // 是否关闭日志，默认不关闭
    close_log_ = 0;
    // 并发模型选择，默认是 proactor
    actor_mode_ = 0;

}
