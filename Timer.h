//
// Created by zhujiaying on 2021/6/23.
//

#ifndef MYWEBSERVER_TIMER_H
#define MYWEBSERVER_TIMER_H

#include <sys/time.h>
#include <memory>

class Timer{
public:
    typedef std::shared_ptr<Timer> spTimer;
    Timer(int fd, int timeout);

    ~Timer();

    bool isTimeout();

    void update(int timeout);

    int getfd(){ return _fd; }

    spTimer next;

    spTimer last;

private:
    int _fd;

    int _timeout;

    struct timeval _now;

};

#endif //MYWEBSERVER_TIMER_H
