//
// Created by yuanh on 2021/7/2.
//

#include "config.h"

int main(int argc, char **argv)
{
    // todo: 建立一个配置系统，通过 json 文件来进行参数配置
    Config config;
    config.ParseConfig(argc, argv);

    WebServer server;
    server.init(config.port_);

    server.start();

}