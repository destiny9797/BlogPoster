//
// Created by zhujiaying on 2021/6/19.
//

#include "TcpServer.h"
#include "TaskPool.h"
#include "Connection.h"
#include "Log.h"

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
#define TIMEOUT 120000 //120s

TcpServer::TcpServer(int port, spTaskPool taskpool)
    : _port(port),
      _taskpool(taskpool),
      _quit(false),
      _threadpool(4, _taskpool),
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

    LOG_INFO("===========ZhuJiaying's Simple WebServer===========");
    LOG_INFO("Open port: %d", port);
    LOG_INFO("IO Model: Epoll(ET)");
    LOG_INFO("Connection: depend on client");
    LOG_INFO("Available Max Open files: %d", MAX_CONN);
    LOG_INFO("===================================================");

}


TcpServer::~TcpServer() {
    if (close(_epoll_fd)<0){
        perror("close epoll fd wrong");
    }
    if (close(_serv_sock)<0){
        perror("close serv socket wrong");
    }
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
    if ((clnt_sock = accept(_serv_sock, (struct sockaddr*)&clnt_addr, &socklen)) > 0){
        //添加HttpConnection
        spConnection conn = std::make_shared<Connection>(clnt_sock,inet_ntoa(clnt_addr.sin_addr));
        conn->setHandleRead(std::bind(&TcpServer::handleRequest, this, clnt_sock));
        conn->setHandleWrite(std::bind(&TcpServer::handleWrite, this, clnt_sock));
        conn->setHandleError(std::bind(&TcpServer::handleError, this, clnt_sock));

        {
            //这一组操作应该为原子操作，或者把accept也放到工作线程中
            //其实也没必要，因为注册到epoll之前是不会有别的线程来操作这个套接字的
            std::unique_lock<std::mutex> lk(*conn->getMutex());
            _connections[clnt_sock] = conn;
            _timermanager.addTimer(clnt_sock, TIMEOUT);

            //设置为非阻塞
            setNonBlocking(clnt_sock);
            ctlEpoll(clnt_sock);
        }
        LOG_INFO("New Connection From: %s, Socket=%d", inet_ntoa(clnt_addr.sin_addr), clnt_sock);
//        std::cout << "New Connection from: " << inet_ntoa(clnt_addr.sin_addr) << std::endl;

//        std::make_shared<Request>();
//        std::cout << "clnt_sock=" << clnt_sock << ", conn=" << _connections[clnt_sock] << std::endl;
//        _handleNewConn(clnt_sock);
    }

}

void TcpServer::handleClose(int fd) {
    //清除连接，清除定时器，还要记得从epoll中取消注册
//    if (_connections[fd] == nullptr){
//        return;
//    }
    spConnection conn = _connections[fd];
//    assert(conn!=nullptr);
    if (conn==nullptr){
        return;
    }
    std::unique_lock<std::mutex> lk(*conn->getMutex());
    _timermanager.rmTimer(fd);
    _connections[fd] = nullptr;
    const char* addr = conn->getAddr();
    conn->Clear(); //bug: 以防系统没有来得及回收内存
    delEpoll(fd);
    close(fd);
    LOG_DEBUG("Socket %d(%s): closed", fd, addr);
//    std::cout << "Socket " << fd << " Closed!" << std::endl;
}

void TcpServer::handleExpire(int fd) {
    //主线程调用，如果正好有线程在写或读这个fd怎么办？
    //没关系，智能指针的读写加锁了，如果已关闭IO会返回-1
//    LOG_DEBUG("Socket %d(%s): expired.", fd, addr);
    handleClose(fd);
}

void TcpServer::handleRequest(int fd) {
//    int fd = conn->getfd();
    spConnection conn = _connections[fd];
//    assert(conn!=nullptr);
    if (conn==nullptr){
        return;
    }
    {
        std::unique_lock<std::mutex> lk(*conn->getMutex());
        //双重锁，这一层用于：主线程和工作线程同时操作fd的定时器
        //函数里的那一层用于：多个工作线程同时操作定时器链表
        _timermanager.updateTimer(fd, TIMEOUT);
    }

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


    //业务处理
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
//    assert(conn!=nullptr);
    if (conn==nullptr){
        return;
    }
    {
        std::unique_lock<std::mutex> lk(*conn->getMutex());
        _timermanager.updateTimer(fd, TIMEOUT); //写也更新定时器，因为表示对方在接受
    }

    int writeerr;
    int nwrite = conn->Write(writeerr);
    if (nwrite < 0){
        if (writeerr == EAGAIN){
            //没发完,此时不允许收
            uint32_t ev = EPOLLOUT | EPOLLET | EPOLLONESHOT;
            modEpoll(fd, ev);
            LOG_WARN("Socket %d(%s): Write unfinish to", fd, conn->getAddr());
        }
        else if (writeerr == EPIPE){
            LOG_WARN("Socket %d(%s): Write wrong, connection is closed by peer", fd, conn->getAddr());
            std::cout << "rst" << std::endl;
            handleClose(fd);
        }
        else{
            LOG_WARN("Socket %d(%s): Write wrong to ", fd, conn->getAddr());
            handleClose(fd);
        }
    }
    else{
        //发完了，或者本来就没有消息要发
        if (conn->isKeepalive()){
            uint32_t ev = EPOLLIN | EPOLLET | EPOLLONESHOT;
            modEpoll(fd, ev);
            LOG_WARN("Socket %d(%s): Write finish to , keepalive, wait for read", fd, conn->getAddr());
        }
        else{
            handleClose(fd);
        }

    }
}

void TcpServer::handleError(int fd) {
//    int fd = conn->getfd();
    handleClose(fd);
}



void TcpServer::Start() {
    LOG_INFO("Now Start!");
    _threadpool.run();
    struct epoll_event ev[MAX_CONN];
    while (!_quit){
        _timermanager.closeExpire();
        int event_cnt = epoll_wait(_epoll_fd, ev, MAX_CONN, TIMEOUT/10);
        if (event_cnt < 0){
            if (errno==EINTR){
                LOG_ERROR("Outside Interrupt!");
            }
            else{
                LOG_ERROR("Epoll_wait wrong!");
            }
        }

        for (int i=0; i<event_cnt; ++i){
            int evfd = ev[i].data.fd;
            uint32_t curev = ev[i].events;
            if (evfd == _serv_sock){
                //new connection
                handleNewConn();
            }
            else{
                //new event [ EPOLLIN | EPOLLOUT ]
                _connections[evfd]->setEvent(curev);
                _taskpool->addTask(_connections[evfd]);
            }
        }
    }
    //等服务器退出，日志才退出
    LOG_INFO("TcpServer Quit");
    Log::getInstance().Quit();
}


void TcpServer::Quit() {
    //等待工作线程都退出才退出
    _threadpool.quit();
//    _taskpool->notifyAll();
    _threadpool.join();

    _quit = true;
}