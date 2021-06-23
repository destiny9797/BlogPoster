//
// Created by zhujiaying on 2021/6/23.
//

#ifndef MYWEBSERVER_TIMER_H
#define MYWEBSERVER_TIMER_H

#include <sys/time.h>

class Timer{
public:
    Timer(int fd, int timeout);

    ~Timer();

    bool isTimeout();

    void update(int timeout);

    int getfd(){ return _fd; }

private:
    int _fd;

    int _timeout;

    struct timeval _now;
};

#endif //MYWEBSERVER_TIMER_H
