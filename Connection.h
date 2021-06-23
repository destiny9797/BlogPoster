//
// Created by zhujiaying on 2021/6/21.
//

#ifndef MYWEBSERVER_CONNECTION_H
#define MYWEBSERVER_CONNECTION_H

#include <string>
#include <functional>

class Connection{
public:
    typedef std::function<void(void)> callback;

    Connection(int fd);

    ~Connection();

public:
    void setEvent(uint32_t ev) { _event = ev; }

    uint32_t getEvent(){ return _event; }

    int getfd(){ return _fd; }

    void setMsg(std::string msg){ _msgtosend = msg; }

    void getMsg(std::string& msg){ msg = _msgtosend; }

    void setHalfclosed(){ _halfclosed = true;}

    bool getHalfclosed(){ return _halfclosed; }

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


private:
    int _fd;

    uint32_t _event;

    std::string _msgtosend;

    bool _halfclosed;


    callback handleRead;

    callback handleWrite;

    callback handleError;
};

#endif //MYWEBSERVER_CONNECTION_H
