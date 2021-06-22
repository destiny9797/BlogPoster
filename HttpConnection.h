//
// Created by zhujiaying on 2021/6/21.
//

#ifndef MYWEBSERVER_HTTPCONNECTION_H
#define MYWEBSERVER_HTTPCONNECTION_H

#include <string>

class HttpConnection{
public:
    HttpConnection(int fd);

    ~HttpConnection();

public:
    void setEvent(uint32_t ev) { _event = ev; }

    uint32_t getEvent(){ return _event; }

    int getfd(){ return _fd; }

    void setMsg(std::string msg){ _msgtosend = msg; }

    void getMsg(std::string& msg){ msg = _msgtosend; }

    void setHalfclosed(){ _halfclosed = true;}

    bool getHalfclosed(){ return _halfclosed; }


private:
    int _fd;

    uint32_t _event;

    std::string _msgtosend;

    bool _halfclosed;
};

#endif //MYWEBSERVER_HTTPCONNECTION_H
