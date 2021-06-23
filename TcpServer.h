//
// Created by zhujiaying on 2021/6/19.
//

#ifndef MYWEBSERVER_TCPSERVER_H
#define MYWEBSERVER_TCPSERVER_H

#include "ThreadPool.h"
//#include "HttpParser.h"
#include <memory>
#include <functional>
#include <string>

class TaskPool;
class Connection;

class TcpServer{
public:
    typedef std::shared_ptr<TaskPool> spTaskPool;
    typedef std::shared_ptr<Connection> spConnection;
    typedef std::function<int(std::string&, std::string&)> callback_parse;


    TcpServer(int port, spTaskPool taskpool);

    ~TcpServer();

    void Start();

    void Quit();

    void ctlEpoll(int fd);

    void modEpoll(int fd, uint32_t ev);

    void setNonBlocking(int fd);

    void handleNewConn();

    void handleClose(int fd);

    void handleRequest(int fd);

    void handleWrite(int fd);

    void handleError(int fd);

    int readMessage(int fd, std::string& msg);

    int sendMessage(int fd, const std::string& msg);

    void setParseRequest(const callback_parse& cb){
        _parseRequest = cb;
    }

private:

    std::vector<spConnection> _connections;

    spTaskPool _taskpool;

    ThreadPool _threadpool;

    int _serv_sock;

    int _port;

    int _epoll_fd;

    bool _quit;


    callback_parse _parseRequest;

};

#endif //MYWEBSERVER_TCPSERVER_H
