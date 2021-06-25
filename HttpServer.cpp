//
// Created by zhujiaying on 2021/6/19.
//

#include "HttpServer.h"
#include "TcpServer.h"
#include "TaskPool.h"
#include "Connection.h"
#include "HttpParser.h"

#include <string.h>
#include <iostream>
#include <functional>
#include <fstream>
#include <sstream>


HttpServer::HttpServer(int port)
    : _taskpool(std::make_shared<TaskPool>()),
      _tcpserver(std::make_shared<TcpServer>(port, _taskpool))
{
//    _tcpserver->setParseRequest(std::bind(&HttpServer::ParseRequest, this, std::placeholders::_1, std::placeholders::_2));
}

HttpServer::~HttpServer() {
    std::cout << "~HttpServer()" << std::endl;
}



void HttpServer::Start() {
//    _threadpool.run();
    _tcpserver->Start();
}

void HttpServer::Quit() {
    _tcpserver->Quit();
//    _threadpool.quit();
}
