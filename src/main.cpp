//
// Created by yuanh on 2021/4/2.
//

#include "config/config.h"
#include "webserver/webserver.h"

int main(int argc, char **argv)
{
    // 服务器端的数据库登录名，密码，库名
    string user = "root";
    string passwd = "123456";
    string database_name = "webservertest";
    // todo: 建立一个配置系统，通过 json 文件来进行参数配置
    Config config;
    config.ParseConfig(argc, argv);

    WebServer server;

    server.Init(config.port_, user, passwd, database_name, config.log_write_, config.opt_linger_, config.trig_mode_,
                config.sql_num_, config.thread_num_, config.close_log_, config.actor_mode_);


}