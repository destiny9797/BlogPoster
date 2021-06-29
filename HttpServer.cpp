//
// Created by zhujiaying on 2021/6/19.
//

#include "HttpServer.h"
#include "TcpServer.h"
#include "TaskPool.h"
#include "Connection.h"
#include "HttpParser.h"
#include "Log.h"
#include <unistd.h>

std::string HttpServer::_homepage = "index.html";

std::string HttpServer::getPath() {
    char* curpath = getcwd(NULL, 0);
    if (curpath==nullptr){
        perror("getcwd wrong:");
        exit(1);
    }
    else{
       return std::string(curpath) + "/../resourses";
    }
}

std::string HttpServer::getHomepage() {
    return _homepage;
}


HttpServer::HttpServer(int port, const std::string& homepage)
    : _taskpool(std::make_shared<TaskPool>())
{
    Log& log = Log::getInstance();
    log.Init("./log", 1024);
    Log::getInstance().Start(); //测试时不打开

    _tcpserver = std::make_shared<TcpServer>(port, _taskpool);
    HttpServer::_homepage = homepage;
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
