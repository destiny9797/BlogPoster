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

HttpServer & HttpServer::getInstance() {
    static HttpServer instance;
    return instance;
}

std::string HttpServer::getPath() {
    char* curpath = getcwd(NULL, 0);
    if (curpath==nullptr){
        perror("getcwd wrong:");
        exit(1);
    }
    else{
       return std::string(curpath) + "/../resourse";
    }
}

std::string HttpServer::getHomepage() {
    return _homepage;
}


HttpServer::HttpServer()
    : _taskpool(std::make_shared<TaskPool>()),
      _inited(false)
{

}

HttpServer::~HttpServer() {
//    std::cout << "~HttpServer()" << std::endl;
}

void HttpServer::Init(int port, const std::string &homepage) {
    Log& log = Log::getInstance();
    log.Init("./log", 1024);
//    Log::getInstance().Start(); //测试时不打开

    _tcpserver = std::make_shared<TcpServer>(port, _taskpool);
    HttpServer::_homepage = homepage;
    _inited = true;
}


void HttpServer::Start() {
    if (!_inited){
        std::cout << "HttpServer is not init" << std::endl;
        exit(1);
    }
    _tcpserver->Start();
}

void HttpServer::Quit() {
    if (_inited){
        _tcpserver->Quit();
    }
}
