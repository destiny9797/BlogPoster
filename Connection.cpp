//
// Created by zhujiaying on 2021/6/21.
//

#include "Connection.h"
#include "Log.h"
#include <iostream>
#include <sys/epoll.h>
#include <assert.h>
#include <sys/sendfile.h>
#include <cstring>

#define BUF_SIZE 1024

Connection::Connection(int fd, const char* addr)
    : _fd(fd),
      _halfclosed(false),
      _inbuffer(BUF_SIZE),
      _outbuffer(BUF_SIZE),
      _parser(),
      _responser(),
      _filefd(0),
      _keepalive(false),
      _code(200),
      _addr(addr)
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
            break;
        }
        total += nread;
        LOG_DEBUG("Socket %d(%s): Read %d bytes from", _fd, _addr, nread);
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
        LOG_DEBUG("Socket %d(%s): Write %d bytes to", _fd, _addr, nwrite);
    }
    assert(_outbuffer.datasize()==0);

    //...
    int filefd = _responser.getFile();
    if (filefd > 0){
        LOG_DEBUG("Socket %d(%s): request for file %s", _fd, _addr, _parser.getUrl().c_str());
        off_t offset = _responser.getOffset();
        while (_responser.getOffset()<_responser.getFileSize()){
            int nsend = sendfile(_fd, filefd, &offset, _responser.getFileSize()-_responser.getOffset());
            _responser.setOffset(offset);
            if (nsend < 0){
                err = errno;
                return -1;
            }
            LOG_DEBUG("Socket %d(%s): Sendfile %d bytes to", _fd, _addr, nsend);
        }
//        std::cout << "offset=" << _responser.getOffset() << ", filesize=" << _responser.getFileSize() << std::endl;
        assert(_responser.getOffset()==_responser.getFileSize());
    }

    _parser.init();
    _responser.init();

    return 1;
}

bool Connection::processCore() {
    HTTP_CODE httpcode = _parser.parseContent(_inbuffer);
    if (httpcode == GET_REQUEST){
//        if (_parser.getMethod()=="POST"){
//            _parser.parsePost();
//        }
        _responser.setStatus(_code, _parser.getUrl());
        if (_parser.isKeepalive()){
            _keepalive = true;
        }
    }
    else if (httpcode == NO_REQUEST){
        return false;
    }
    _responser.setResponse(_outbuffer);
//    std::cout << "After preocess, outbuffer:" << std::endl;
//    _outbuffer.printdata();

    return true;
}


void Connection::handleEvent() {
    if (_event & (EPOLLIN)){ //对方有数据或正常关闭
        handleRead();
    }
    else if (_event & EPOLLOUT){ //缓冲区可写
        handleWrite();
    }
    else if (_event & (EPOLLERR | EPOLLHUP)){
        handleError();
    }
}


void Connection::Clear() {
    _inbuffer.init();
    _outbuffer.init();
    _parser.init();
    _responser.init();
    _keepalive = false;
    _code = 200;
}