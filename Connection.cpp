//
// Created by zhujiaying on 2021/6/21.
//

#include "Connection.h"
#include <iostream>
#include <sys/epoll.h>


Connection::Connection(int fd) : _fd(fd), _halfclosed(false) {

}

Connection::~Connection() {
//    std::cout << "~Connection()" << std::endl;
}


void Connection::handleEvent() {
    if (_event & (EPOLLIN | EPOLLPRI)){ //对方有数据或正常关闭
//        std::cout << "one thread handle read" << std::endl;
        handleRead();
    }
    else if (_event & EPOLLOUT){
//        std::cout << "one thread handle write" << std::endl;
        handleWrite();
    }
    else if (_event & (EPOLLERR | EPOLLHUP)){
//        std::cout << "one thread handle error" << std::endl;
        handleError();
    }
}
