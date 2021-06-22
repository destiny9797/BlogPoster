//
// Created by zhujiaying on 2021/6/19.
//

#ifndef MYWEBSERVER_TCPSERVER_H
#define MYWEBSERVER_TCPSERVER_H

#include "ThreadPool.h"
#include "HttpParser.h"
#include <memory>
#include <functional>
#include <string>

class TaskPool;
class HttpConnection;

class TcpServer{
public:
    typedef std::shared_ptr<TaskPool> spTaskPool;
    typedef std::shared_ptr<HttpConnection> spHttpConnection;
    typedef std::function<HTTP_CODE(std::string&, std::string&)> callback_parse;

    TcpServer(int port, spTaskPool taskpool);

    ~TcpServer();

    void Start();

    void Quit();

    int getServsock(){ return _serv_sock; }

    void ctlEpoll(int fd);

    void modEpoll(int fd, uint32_t ev);

    void handleNewConn();

    void handleClose(int fd);

    void handleRequest(spHttpConnection);

    void handleWrite(spHttpConnection);

    void handleError(spHttpConnection);

    int readMessage(int fd, std::string& msg);

    int sendMessage(int fd, const std::string& msg);

    void setParseRequest(const callback_parse& cb){
        _parseRequest = cb;
    }

private:

    std::vector<spHttpConnection> _connections;

    spTaskPool _taskpool;

    ThreadPool _threadpool;

    int _serv_sock;

    int _port;

    int _epoll_fd;

    bool _quit;


    callback_parse _parseRequest;

};

#endif //MYWEBSERVER_TCPSERVER_H
