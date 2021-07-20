cmake_minimum_required(VERSION 3.17)

project(mix_webserver)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_CXX_FLAGS "$ENV{CXXFLAS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

# todo 将手动添加源文件改为自动添加
#AUX_SOURCE_DIRECTORY(src src)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories("/usr/include/mysql")

add_executable(server
        src/main.cpp
        src/config/config.cpp
        src/config/config.h
        src/webserver/webserver.h
        src/webserver/webserver.cpp
        src/http/http_conn.h
        src/block_queue/block_queue.h
        src/timer/list_timer.h
        src/mysql/sql_conn_pool.h src/log/log.cpp src/mysql/sql_conn_pool.cpp src/http/http_conn.cpp src/util/util.h src/timer/list_timer.cpp src/util/util.cpp)
add_executable(test src/test/unit_test.cpp)

set(MYSQL_LIBS mysqlclient pthread z m rt atomic ssl crypto dl)

target_link_libraries(server ${MYSQL_LIBS})