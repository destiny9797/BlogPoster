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

#define MAX_BUF 4096
#define MAX_CONN 1000

TcpServer::TcpServer(int port, spTaskPool taskpool)
    : _port(port),
      _taskpool(taskpool),
      _quit(false),
      _threadpool(8, _taskpool),
      _connections(MAX_CONN, 0){
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

    if (listen(_serv_sock, 10) < 0){
        perror("listen wrong.");
        exit(1);
    }

    _epoll_fd = _epoll_fd = epoll_create(10);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = _serv_sock;
    //设置为非阻塞
    setNonBlocking(_serv_sock);
    epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _serv_sock, &event);

//    _threadpool.setHandleRead(std::bind(&TcpServer::handleRequest, this, std::placeholders::_1));
//    _threadpool.setHandleWrite(std::bind(&TcpServer::handleWrite, this, std::placeholders::_1));
//    _threadpool.setHandleError(std::bind(&TcpServer::handleError, this, std::placeholders::_1));
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

void TcpServer::setNonBlocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void TcpServer::handleNewConn() {
    struct sockaddr_in clnt_addr;
    socklen_t socklen;
    int clnt_sock;
    while ((clnt_sock = accept(_serv_sock, (struct sockaddr*)&clnt_addr, &socklen)) > 0){
        //设置为非阻塞
        setNonBlocking(clnt_sock);
        ctlEpoll(clnt_sock);
        std::cout << "New Connection from: " << inet_ntoa(clnt_addr.sin_addr) << std::endl;

        //添加HttpConnection
        spConnection conn = std::make_shared<Connection>(clnt_sock);
        conn->setHandleRead(std::bind(&TcpServer::handleRequest, this, clnt_sock));
        conn->setHandleWrite(std::bind(&TcpServer::handleWrite, this, clnt_sock));
        conn->setHandleError(std::bind(&TcpServer::handleError, this, clnt_sock));
        _connections[clnt_sock] = conn;
//        std::make_shared<Request>();
        std::cout << "clnt_sock=" << clnt_sock << ", conn=" << _connections[clnt_sock] << std::endl;
//        _handleNewConn(clnt_sock);
    }

}

void TcpServer::handleClose(int fd) {
    _connections[fd] = 0;
    close(fd);
    std::cout << "Closed!" << std::endl;
}

void TcpServer::handleRequest(int fd) {
//    int fd = conn->getfd();
    spConnection conn = _connections[fd];
    //读数据
    std::string msg_recv;
    std::string new_response;

    while (1){
        std::string msg;
        int nread = readMessage(fd, msg);
        if (nread == 0){
            handleClose(fd);
            break;
        }
        else if (nread < 0){
            break;
        }
        msg_recv += msg;
        if (msg_recv.empty()){
            break;
        }

        //http解析
        std::cout << "nread=" << nread << ", received msg=" << msg_recv << std::endl;
        int parsed_len = _parseRequest(msg_recv, new_response);

        //如果解析成功，
        if (parsed_len > 0){
            std::string msg_response;
            conn->getMsg(msg_response);
            msg_response += new_response;
            std::cout << "msg_response=" << msg_response << std::endl;

            if (!msg_response.empty()){
                int nsend = sendMessage(fd, msg_response);
                //如果没写完，注册写事件
                if (nsend < msg_response.length()) {
                    msg_response.erase(msg_response.begin(), msg_response.begin() + nsend);
                    conn->setMsg(msg_response);

                    uint32_t ev = EPOLLIN | EPOLLET | EPOLLOUT | EPOLLONESHOT;
                    modEpoll(fd, ev);
                }
                else{
                    uint32_t ev = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    modEpoll(fd, ev);
                }
            }
        }

        //防止粘包
        if (parsed_len == msg_recv.length()){
            break;
        }
        else{
            msg_recv.erase(msg_recv.begin(),msg_recv.begin()+parsed_len);
        }

    }



}

void TcpServer::handleWrite(int fd) {
//    int fd = conn->getfd();
    spConnection conn = _connections[fd];
    std::string msg;
    conn->getMsg(msg);
    if (msg.empty()){
        return;
    }
    int nsend = sendMessage(fd, msg);
    if (nsend < 0){
        perror("write wrong.");
    }
    else if (nsend < msg.length()){ //如果没写完，注册写事件
        msg.erase(msg.begin(), msg.begin() + nsend);
        conn->setMsg(msg);

        uint32_t ev = EPOLLIN | EPOLLET | EPOLLOUT | EPOLLONESHOT;
        modEpoll(fd, ev);
    }
    else{
        //如果写完了，就重置ONESHOT
        msg.clear();
        conn->setMsg(msg);
        uint32_t ev = EPOLLIN | EPOLLET | EPOLLONESHOT;
        modEpoll(fd, ev);
        if (conn->getHalfclosed()){
            handleClose(fd);
        }
    }
}

void TcpServer::handleError(int fd) {
//    int fd = conn->getfd();
    handleClose(fd);
}

int TcpServer::readMessage(int fd, std::string& msg) {
    char buf[MAX_BUF];
    int readsum = 0;
    while (1){
        int nread = 0;
        if ((nread=read(fd, buf, MAX_BUF)) > 0){
            msg += std::string(buf);
            msg.append(buf, nread);
            readsum += nread;
        }
        else if (nread==0){
            //对方 closed connection
            return 0;
        }
        else{
            //no more data
            if (readsum==0){
                return -1;
            }
            else
                return readsum;
        }
    }
}

int TcpServer::sendMessage(int fd, const std::string &msg) {
    int len = msg.size();
    const char* buf = msg.c_str();
    int sendsum = 0;
    while (sendsum<len){
        int ntosend = std::min(len-sendsum,MAX_BUF);
        int nsend = write(fd,buf+sendsum,ntosend);
        if (nsend < 0){
            break;
        }
        sendsum += nsend;
    }
    std::cout << sendsum << " Bytes Send! Total msg is " << len <<" Bytes." << std::endl;

    return sendsum;
}


void TcpServer::Start() {
    _threadpool.run();
    while (!_quit){
        struct epoll_event ev[10];
        int event_cnt = epoll_wait(_epoll_fd, ev, 10, -1);
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
                if (curev & EPOLLIN){
                    std::cout << "EPOLLIN" << std::endl;
                }
                if (curev & EPOLLOUT){
                    std::cout << "EPOLLOUT" << std::endl;
                }
                if (curev & EPOLLERR){
                    std::cout << "EPOLLERR" << std::endl;
                }
                if (curev & EPOLLHUP){
                    std::cout << "EPOLLHUP" << std::endl;
                }
                std::cout << "evfd=" << evfd << ", _connection[evfd]=" << _connections[evfd] << std::endl;
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