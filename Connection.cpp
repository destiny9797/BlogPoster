//
// Created by zhujiaying on 2021/6/21.
//

#include "Connection.h"
#include "Log.h"
#include <iostream>
#include <sys/epoll.h>
#include <assert.h>
#include <sys/sendfile.h>

#define BUF_SIZE 1024

Connection::Connection(int fd)
    : _fd(fd),
      _halfclosed(false),
      _inbuffer(BUF_SIZE),
      _outbuffer(BUF_SIZE),
      _parser(),
      _responser(),
      _filefd(0),
      _keepalive(false)
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
        std::cout << "After receive, " << std::endl;
        _inbuffer.printdata();
        if (nread <= 0){
            //wrong, or no more data, or closed
//            std::cout << "return" << std::endl;
            break;
        }
        total += nread;
        LOG_DEBUG("Read %d bytes from Socket %d", nread, _fd);
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
        LOG_DEBUG("Write %d bytes to Socket %d", nwrite, _fd);
    }
    assert(_outbuffer.datasize()==0);

    //...
    int filefd = _responser.getFile();
    if (filefd > 0){
        LOG_DEBUG("Socket %d: request for file %s", _fd, _parser.getUrl().c_str());
        off_t offset = _responser.getOffset();
        while (_responser.getOffset()<_responser.getFileSize()){
            int nsend = sendfile(_fd, filefd, &offset, _responser.getFileSize()-_responser.getOffset());
            _responser.setOffset(offset);
            if (nsend < 0){
                err = errno;
                return -1;
            }
            LOG_DEBUG("Sendfile %d bytes to Socket %d", nsend, _fd);
        }
        std::cout << "offset=" << _responser.getOffset() << ", filesize=" << _responser.getFileSize() << std::endl;
        assert(_responser.getOffset()==_responser.getFileSize());
    }

    _parser.init();
    _responser.init();

    return 1;
}

bool Connection::processCore() {
    HTTP_CODE httpcode = _parser.parseContent(_inbuffer);
    if (httpcode == GET_REQUEST){
        _responser.setStatus(_parser.getMethod(), _parser.getUrl());
        if (_parser.isKeepalive()){
            _keepalive = true;
        }
//        std::cout << "httpcode=GET_REQUEST" << std::endl;
    }
    else if (httpcode == NO_REQUEST){
//        std::cout << "httpcode=NO_REQUEST" << std::endl;
        return false;
    }
    _responser.setResponse(_outbuffer);
//    std::cout << "After preocess, outbuffer:" << std::endl;
//    _outbuffer.printdata();

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


void Connection::Clear() {
    _inbuffer.init();
    _outbuffer.init();
    _parser.init();
    _responser.init();
    _keepalive = false;
}