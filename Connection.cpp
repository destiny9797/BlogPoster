//
// Created by zhujiaying on 2021/6/21.
//

#include "Connection.h"
#include <iostream>
#include <sys/epoll.h>
#include <assert.h>


Connection::Connection(int fd)
    : _fd(fd),
      _halfclosed(false),
      _inbuffer(),
      _outbuffer(),
      _parser(),
      _responser()
{

}

Connection::~Connection() {
//    std::cout << "~Connection()" << std::endl;
}

int Connection::Read(int& err) {
    int total = 0;
    do{
//        std::cout << "Before receive, " << std::endl;
//        _inbuffer.printdata();
        int nread = _inbuffer.Read(_fd, err);
//        std::cout << "After receive, " << std::endl;
//        _inbuffer.printdata();
        if (nread <= 0){
            //wrong, or no more data, or closed
//            std::cout << "return" << std::endl;
            break;
        }
        total += nread;
    }while(1);
    return total;
    //.......

}

int Connection::Write(int& err) {
    while (_outbuffer.datasize() > 0){
//        std::cout << "Before send, " << std::endl;
//        _outbuffer.printdata();
        int nwrite = _outbuffer.Write(_fd, err);
//        std::cout << "After send, " << std::endl;
//        _outbuffer.printdata();
        if (nwrite < 0){
            //没发完，设置EPOLLOUT
            return -1;
        }
    }
    assert(_outbuffer.datasize()==0);
    return 1;
}

bool Connection::processCore() {
    HTTP_CODE httpcode = _parser.parseContent(_inbuffer);
    if (httpcode == GET_REQUEST){
        _responser.setStatus(200, "");
//        std::cout << "httpcode=GET_REQUEST" << std::endl;
    }
    else if (httpcode == NO_REQUEST){
//        std::cout << "httpcode=NO_REQUEST" << std::endl;
        return false;
    }
    _responser.setResponse(_outbuffer);
//    std::cout << "After preocess, outbuffer:" << std::endl;
//    _outbuffer.printdata();
    _parser.init();
    return true;
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


