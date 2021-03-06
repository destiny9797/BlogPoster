//
// Created by zhujiaying on 2021/6/21.
//

#ifndef MYWEBSERVER_CONNECTION_H
#define MYWEBSERVER_CONNECTION_H

#include "Buffer.h"
#include "HttpParser.h"
#include "HttpResponse.h"
#include <string>
#include <functional>
#include <mutex>
#include <arpa/inet.h>


class Connection{
public:

    typedef std::function<void(void)> callback;

    Connection(int fd, const char* addr);

    ~Connection();

    bool isKeepalive(){ return _keepalive; }

    const char* getAddr(){ return _addr; }

public:
    void setEvent(uint32_t ev) { _event = ev; }

//    uint32_t getEvent(){ return _event; }

    int Read(int& err);

    int Write(int& err);


    void handleEvent();

    void setHandleRead(const callback& cb){
        handleRead = cb;
    }

    void setHandleWrite(const callback& cb){
        handleWrite = cb;
    }

    void setHandleError(const callback& cb){
        handleError = cb;
    }

    bool processCore();

    std::mutex* getMutex(){ return &_mutex; }

    void Clear();

private:
    void handlePost(const std::string& post);

    std::mutex _mutex;

    int _fd;

    uint32_t _event;

    const char* _addr;


    Buffer _inbuffer;

    Buffer _outbuffer;

    HttpParser _parser;

    HttpResponse _responser;

    int _filefd;

    bool _halfclosed;

    bool _keepalive;

    int _code;


    callback handleRead;

    callback handleWrite;

    callback handleError;
};

#endif //MYWEBSERVER_CONNECTION_H
