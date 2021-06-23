cmake_minimum_required(VERSION 3.11)
project(MyWebServer)

set(CMAKE_CXX_STANDARD 14)

set(SRCS
        HttpServer.cpp
        TcpServer.cpp
        Connection.cpp
        ThreadPool.cpp
        TaskPool.cpp
        HttpParser.cpp
        main.cpp)

add_executable(MyWebServer ${SRCS})

target_link_libraries(MyWebServer pthread)