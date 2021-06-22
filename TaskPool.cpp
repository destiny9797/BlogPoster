//
// Created by zhujiaying on 2021/6/20.
//

#include "TaskPool.h"
#include <iostream>

TaskPool::TaskPool() {

}

TaskPool::~TaskPool() {
    std::cout << "~TaskPool()" << std::endl;
}

void TaskPool::addTask(TaskPool::spHttpConnection conn) {
    std::unique_lock<std::mutex> lk(_mutex);
    _activeconn.push(conn);
//    _cond.notify_one();
}

TaskPool::spHttpConnection TaskPool::getTask() {
    std::unique_lock<std::mutex> lk(_mutex);
    spHttpConnection t = nullptr;
    if (_activeconn.empty()){
//        _cond.wait(lk);
    }
    else{
        t = _activeconn.front();
        _activeconn.pop();
    }
    return t;
}