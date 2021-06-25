//
// Created by zhujiaying on 2021/6/19.
//

#include "TcpServer.h"
#include "TaskPool.h"
#include "Connection.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <errno.h>

//#define MAX_BUF 4096
#define MAX_CONN 65536
#define TIMEOUT 2000 //2000ms

TcpServer::TcpServer(int port, spTaskPool taskpool)
    : _port(port),
      _taskpool(taskpool),
      _quit(false),
      _threadpool(1, _taskpool),
      _connections(MAX_CONN, 0),
      _timermanager(std::bind(&TcpServer::handleExpire, this, std::placeholders::_1))
{
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(_port);

    _serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (_serv_sock < 0){
        perror("socket wrong.");
        exit(1);
    }

    if (bind(_serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        perror("bind wrong.");
        exit(1);
    }

    if (listen(_serv_sock, MAX_CONN) < 0){
        perror("listen wrong.");
        exit(1);
    }

    _epoll_fd = _epoll_fd = epoll_create(MAX_CONN);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = _serv_sock;
    //设置为非阻塞
    setNonBlocking(_serv_sock);
    epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _serv_sock, &event);

}


TcpServer::~TcpServer() {
    close(_serv_sock);
    std::cout << "~TcpServer()" << std::endl;
}

void TcpServer::ctlEpoll(int fd) {
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    event.data.fd = fd;
    epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

void TcpServer::modEpoll(int fd, uint32_t ev) {
    struct epoll_event event;
    event.events = ev;
    event.data.fd = fd;

    epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

void TcpServer::delEpoll(int fd) {

    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

void TcpServer::setNonBlocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void TcpServer::handleNewConn() {
    struct sockaddr_in clnt_addr;
    socklen_t socklen = sizeof(clnt_addr); //bug: 未初始化会收到来自1.0.0.0的连接
    int clnt_sock;
    while ((clnt_sock = accept(_serv_sock, (struct sockaddr*)&clnt_addr, &socklen)) > 0){
        //添加HttpConnection
        spConnection conn = std::make_shared<Connection>(clnt_sock);
        conn->setHandleRead(std::bind(&TcpServer::handleRequest, this, clnt_sock));
        conn->setHandleWrite(std::bind(&TcpServer::handleWrite, this, clnt_sock));
        conn->setHandleError(std::bind(&TcpServer::handleError, this, clnt_sock));
        _connections[clnt_sock] = conn;
        _timermanager.addTimer(clnt_sock, TIMEOUT);

        //设置为非阻塞
        setNonBlocking(clnt_sock);
        ctlEpoll(clnt_sock);
//        std::cout << "New Connection from: " << inet_ntoa(clnt_addr.sin_addr) << std::endl;

//        std::make_shared<Request>();
//        std::cout << "clnt_sock=" << clnt_sock << ", conn=" << _connections[clnt_sock] << std::endl;
//        _handleNewConn(clnt_sock);
    }

}

void TcpServer::handleClose(int fd) {
    //清除连接，清除定时器，还要记得从epoll中取消注册
    if (_connections[fd] == nullptr){
        return;
    }
    _timermanager.rmTimer(fd);
    std::unique_lock<std::mutex> lk(_mutex);
    _connections[fd] = nullptr;
    delEpoll(fd);
    close(fd);
//    std::cout << "Socket " << fd << " Closed!" << std::endl;
}

void TcpServer::handleExpire(int fd) {
    //主线程调用，如果正好有线程在写或读这个fd怎么办？
    //一般来说不会，因为有线程正好在读或写，说明刚更新过定时器，如果都要超时了还没操作完，说明这个连接有问题
    handleClose(fd);
}

void TcpServer::handleRequest(int fd) {
//    int fd = conn->getfd();
    spConnection conn = _connections[fd];
    if (conn==nullptr){
        return;
    }
    _timermanager.updateTimer(fd, TIMEOUT);
    //读数据
    int readerr = 0;
    int nread = conn->Read(readerr);
    if (nread < 0){
        if (readerr != EAGAIN){
//            std::cout << "readerr == EAGAIN" << std::endl;
            handleClose(fd);
        }
    }
    else if (nread == 0){
        //1、如果对方已关闭，直接关闭
        // 2、同样进行剩余消息的发送，如果没消息要发，会断开连接
//        std::cout << "nread == 0" << std::endl;
        handleClose(fd);
        return;
    }

//    std::cout << "before process" << std::endl;
    bool hasRequest = conn->processCore();
    if (!hasRequest){
        return;
    }

    //不管请求是否完整，都尝试发送
//    std::cout << "before handlewrite" << std::endl;
    handleWrite(fd);

}

void TcpServer::handleWrite(int fd) {
//    int fd = conn->getfd();
    spConnection conn = _connections[fd];
    _timermanager.updateTimer(fd, TIMEOUT); //写也更新定时器，因为表示对方在接受

    int writeerr;
    int nwrite = conn->Write(writeerr);
    if (nwrite < 0){
        if (nwrite == EAGAIN){
            //没发完,此时不允许收
            uint32_t ev = EPOLLOUT | EPOLLET | EPOLLONESHOT;
            modEpoll(fd, ev);
        }
        else{
            handleClose(fd);
        }
    }
    else{
        //发完了，或者本来就没有消息要发
        handleClose(fd);
        uint32_t ev = EPOLLIN | EPOLLET | EPOLLONESHOT;
        modEpoll(fd, ev);
    }
}

void TcpServer::handleError(int fd) {
//    int fd = conn->getfd();
    handleClose(fd);
}



void TcpServer::Start() {
    _threadpool.run();
    while (!_quit){
        struct epoll_event ev[10];
        _timermanager.closeExpire();
        int event_cnt = epoll_wait(_epoll_fd, ev, 10, 2000);
        if (event_cnt < 0){
            perror("epoll_wait wrong:");
        }

        for (int i=0; i<event_cnt; ++i){
            int evfd = ev[i].data.fd;
            uint32_t curev = ev[i].events;
            if (evfd == _serv_sock){
                //new connection
                handleNewConn();
            }
            else{
                //new data
//                _addActivateConn(evfd, curev);
                _connections[evfd]->setEvent(curev);
//                if (curev & EPOLLIN){
//                    std::cout << "EPOLLIN" << std::endl;
//                }
//                if (curev & EPOLLOUT){
//                    std::cout << "EPOLLOUT" << std::endl;
//                }
//                if (curev & EPOLLERR){
//                    std::cout << "EPOLLERR" << std::endl;
//                }
//                if (curev & EPOLLHUP){
//                    std::cout << "EPOLLHUP" << std::endl;
//                }
//                std::cout << "evfd=" << evfd << ", _connection[evfd]=" << _connections[evfd] << std::endl;
                _taskpool->addTask(_connections[evfd]);
            }
        }
    }
}


void TcpServer::Quit() {
    _threadpool.quit();
//    _taskpool->notifyAll();
    _threadpool.join();
    _quit = true;

}