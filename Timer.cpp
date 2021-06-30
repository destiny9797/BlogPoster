//
// Created by zhujiaying on 2021/6/23.
//

#include "Timer.h"
#include <unistd.h>
#include <iostream>

Timer::Timer(int fd, int timeout)
    : _fd(fd),
      _timeout(timeout),
      next(nullptr),
      last(nullptr)
{
    gettimeofday(&_now, NULL);
}

Timer::~Timer() {

}

void Timer::update(int timeout) {
    _timeout = timeout;
    gettimeofday(&_now, NULL);
}

bool Timer::isTimeout() {
    struct timeval newnow;
    long long int expire_time = _now.tv_sec * 1000 + _now.tv_usec / 1000 + _timeout;
    gettimeofday(&newnow, NULL);
    long long int now_time = newnow.tv_sec * 1000 + newnow.tv_usec / 1000;
    return now_time > expire_time;
}