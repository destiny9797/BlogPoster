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

    static HttpServer& getInstance();

    static std::string getPath();

    static std::string getHomepage();

    void Init(int port, const std::string& homepage);

    void Start();

    void Quit();


private:
    HttpServer();

    ~HttpServer();

    spTaskPool _taskpool;

    spTcpServer _tcpserver;

    static std::string _homepage;

    bool _inited;

};

#endif //MYWEBSERVER_HTTPSERVER_H
