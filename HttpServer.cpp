//
// Created by zhujiaying on 2021/6/19.
//

#include "HttpServer.h"
#include "TcpServer.h"
#include "TaskPool.h"
#include "Connection.h"
#include "HttpParser.h"
#include "Log.h"



HttpServer::HttpServer(int port)
    : _taskpool(std::make_shared<TaskPool>())
{
    Log& log = Log::getInstance();
    log.Init("./log", 1024);
//    Log::getInstance().Start(); //测试时不打开

    _tcpserver = std::make_shared<TcpServer>(port, _taskpool);
}

HttpServer::~HttpServer() {
//    std::cout << "~HttpServer()" << std::endl;
}



void HttpServer::Start() {
//    _threadpool.run();
    _tcpserver->Start();
}

void HttpServer::Quit() {
    _tcpserver->Quit();
//    _threadpool.quit();
}
