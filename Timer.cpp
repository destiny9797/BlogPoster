//
// Created by zhujiaying on 2021/6/23.
//

#include "Timer.h"
#include <unistd.h>
#include <iostream>

Timer::Timer(int fd, int timeout)
    : _fd(fd),
      _timeout(timeout)
{
    gettimeofday(&_now, NULL);
}

Timer::~Timer() {

}

void Timer::update(int timeout) {
    _timeout = timeout;
//    std::cout << "old_time=" << _now.tv_sec*1000+_now.tv_usec/1000 << std::endl;
    gettimeofday(&_now, NULL);
//    std::cout << "new_time=" << _now.tv_sec*1000+_now.tv_usec/1000 << std::endl;
}

bool Timer::isTimeout() {
    struct timeval newnow;
    long long int expire_time = _now.tv_sec * 1000 + _now.tv_usec / 1000 + _timeout;
    gettimeofday(&newnow, NULL);
    long long int now_time = newnow.tv_sec * 1000 + newnow.tv_usec / 1000;
//    std::cout << "expire_time=" << expire_time << ", now=" << now_time << std::endl;
    return now_time > expire_time;
}