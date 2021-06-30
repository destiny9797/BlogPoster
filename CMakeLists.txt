cmake_minimum_required(VERSION 3.11)
project(BlogPoster)

set(CMAKE_CXX_STANDARD 14)

#FIND_LIBRARY(CONN_LIB libmysqlcppconn8-static.a /usr/local/mysql-connector-c++/lib64/)
#FIND_LIBRARY(SSL_LIB libssl.so.1.1 /usr/lib64/)
#FIND_LIBRARY(CRYPTO_LIB libcrypto.so.1.1 /usr/lib64/)
#
#include_directories("/usr/include/mysql-cppconn-8/")


set(SRCS
        HttpServer.cpp
        TcpServer.cpp
        Connection.cpp
        ThreadPool.cpp
        TaskPool.cpp
        HttpParser.cpp
        HttpResponse.cpp
        Timer.cpp
        TimerManager.cpp
        Buffer.cpp
        Log.cpp
#        MysqlConnection.cpp
#        MysqlConnPool.cpp
        main.cpp)

add_executable(webserver ${SRCS})

#target_link_libraries(BlogPoster pthread mysqlcppconn8 ${SSL_LIB} ${CRYPTO_LIB})

target_link_libraries(webserver pthread)