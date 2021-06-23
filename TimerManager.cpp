//
// Created by zhujiaying on 2021/6/23.
//

#include "TimerManager.h"
#include "Timer.h"
#include <iostream>

TimerManager::TimerManager(int maxfd, const callback& handle)
    : _timers(maxfd, nullptr),
      handleExpire(handle)
{

}

TimerManager::~TimerManager() {

}


void TimerManager::closeExpire(){
    for (spTimer timer : _timers){
        if (timer!=nullptr && timer->isTimeout()){
            //应该在这个线程中关闭吗？
            int fd = timer->getfd();
            handleExpire(fd);
            std::cout << "Socket " << fd << "expired, Closed! " << std::endl;
        }
    }
}

void TimerManager::addTimer(int fd, int timeout) {
    std::unique_lock<std::mutex> lk(_mutex);
    spTimer timer = std::make_shared<Timer>(fd, timeout);
    _timers[fd] = timer;
}

void TimerManager::updateTimer(int fd, int timeout) {
    std::unique_lock<std::mutex> lk(_mutex);
    _timers[fd]->update(timeout);
}

void TimerManager::rmTimer(int fd) {
    std::unique_lock<std::mutex> lk(_mutex);
    _timers[fd] = nullptr;
}