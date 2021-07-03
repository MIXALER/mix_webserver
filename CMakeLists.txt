cmake_minimum_required(VERSION 3.17)

project(mix_webserver)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

set()
#add_library(mix SHARED ${LIB_SRC})
#add_executable(test tests/test.cc)
#add_dependencies(test mix)
#target_link_libraries(test mix)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories("/usr/include/mysql")

add_executable(server tests/test.cc main.cpp config.h webserver.h config.cpp webserver.cpp src/http/http_conn.cpp)

set(MYSQL_LIBS mysqlclient pthread z m rt atomic ssl crypto dl)

target_link_libraries(server ${MYSQL_LIBS})