//
// Created by zhujiaying on 2021/6/20.
//

#include "TaskPool.h"
#include <iostream>

TaskPool::TaskPool() : _quit(false)
{

}

TaskPool::~TaskPool() {
    std::cout << "~TaskPool()" << std::endl;
}

void TaskPool::Quit() {
    _quit = true;
    _cond.notify_all();
}

void TaskPool::addTask(TaskPool::spHttpConnection conn) {
    std::unique_lock<std::mutex> lk(_mutex);
    _activeconn.push(conn);
    _cond.notify_one();
}

TaskPool::spHttpConnection TaskPool::getTask() {
    spHttpConnection t = nullptr;
    std::unique_lock<std::mutex> lk(_mutex);
    _cond.wait(lk,[this](){return !_activeconn.empty() || _quit;});
    if (!_activeconn.empty()){
        t = _activeconn.front();
        _activeconn.pop();
    }
    return t;
}