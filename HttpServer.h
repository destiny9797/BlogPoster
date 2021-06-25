//
// Created by zhujiaying on 2021/6/19.
//

#ifndef MYWEBSERVER_HTTPSERVER_H
#define MYWEBSERVER_HTTPSERVER_H

#include "ThreadPool.h"
//#include "HttpParser.h"

#include <string>
#include <memory>
#include <vector>

class TcpServer;
class TaskPool;

class HttpServer{
public:
    typedef std::shared_ptr<TcpServer> spTcpServer;
    typedef std::shared_ptr<TaskPool> spTaskPool;

    HttpServer(int port);

    ~HttpServer();

    void Start();

    void Quit();


private:

    spTaskPool _taskpool;

    spTcpServer _tcpserver;

};

#endif //MYWEBSERVER_HTTPSERVER_H
