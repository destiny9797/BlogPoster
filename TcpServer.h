//
// Created by zhujiaying on 2021/6/19.
//

#ifndef MYWEBSERVER_TCPSERVER_H
#define MYWEBSERVER_TCPSERVER_H

#include "ThreadPool.h"
#include "TimerManager.h"
#include <memory>
#include <string>
#include <mutex>

class TaskPool;
class Connection;


class TcpServer{
public:
    typedef std::shared_ptr<TaskPool> spTaskPool;
    typedef std::shared_ptr<Connection> spConnection;


    TcpServer(int port, spTaskPool taskpool);

    ~TcpServer();

    void Start();

    void Quit();

    void ctlEpoll(int fd);

    void modEpoll(int fd, uint32_t ev);

    void delEpoll(int fd);

    void setNonBlocking(int fd);

    void handleNewConn();

    void handleClose(int fd);

    void handleRequest(int fd);

    void handleWrite(int fd);

    void handleError(int fd);

    void handleExpire(int fd);

private:
//    std::mutex _mutex;

    std::vector<spConnection> _connections;

    TimerManager _timermanager;

    spTaskPool _taskpool;

    ThreadPool _threadpool;

    int _serv_sock;

    int _port;

    int _epoll_fd;

    bool _quit;


};

#endif //MYWEBSERVER_TCPSERVER_H
