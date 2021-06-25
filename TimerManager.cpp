//
// Created by zhujiaying on 2021/6/23.
//

#include "TimerManager.h"
#include "Timer.h"
#include <iostream>
#include <assert.h>

TimerManager::TimerManager(const callback& handle)
    : _head(std::make_shared<Timer>(-1,-1)),
      _tail(std::make_shared<Timer>(-1,-1)),
      handleExpire(handle),
      _count(0)
{
    _head->next = _tail;
    _tail->last = _head;
}

TimerManager::~TimerManager() {

}


void TimerManager::closeExpire(){
    spTimer timer = _tail->last;
    //尾部是即将超时的
    while (timer!=_head){
        assert(timer!=nullptr);
        if (!timer->isTimeout()){
            break;
        }
        //应该在这个线程中关闭吗？
        int fd = timer->getfd();
        spTimer lasttimer = timer->last;
        handleExpire(fd);
//        std::cout << "Socket " << fd << "expired, Closed! " << std::endl;

        timer = lasttimer;
    }
}

void TimerManager::addTimer(int fd, int timeout) {
    std::unique_lock<std::mutex> lk(_mutex);
    spTimer timer = std::make_shared<Timer>(fd, timeout);
    addToHead(timer);
    _map[fd] = timer;
    _count++;
//    _timers[fd] = timer;
}

void TimerManager::updateTimer(int fd, int timeout) {
    std::unique_lock<std::mutex> lk(_mutex);
    assert(_map.count(fd));
    spTimer timer = _map[fd];
//    assert(timer!=nullptr);
    timer->last->next = timer->next;
    timer->next->last = timer->last;
    addToHead(timer);
//    _timers[fd]->update(timeout);
}

void TimerManager::rmTimer(int fd) {
    std::unique_lock<std::mutex> lk(_mutex);
    assert(_map.count(fd));
    spTimer timer = _map[fd];
//    assert(timer!=nullptr);
    timer->last->next = timer->next;
    timer->next->last = timer->last;
    _map.erase(fd);
    _count--;
//    _timers[fd] = nullptr;
}

void TimerManager::addToHead(spTimer timer) {
    timer->next = _head->next;
    timer->next->last = timer;
    _head->next = timer;
    timer->last = _head;
}