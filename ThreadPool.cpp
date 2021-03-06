//
// Created by zhujiaying on 2021/6/19.
//

#include "ThreadPool.h"
#include "TaskPool.h"
#include "Connection.h"
#include "Log.h"
#include <sys/epoll.h>
#include <iostream>

ThreadPool::ThreadPool(int threadnum, spTaskPool taskpool)
    : _threadnum(threadnum), _taskpool(taskpool), _quit(false){

}

ThreadPool::~ThreadPool() {
    quit();
    std::cout << "~ThreadPool()" << std::endl;
}

void ThreadPool::run() {
    for (int i=0; i<_threadnum; ++i){
        _threadpool.push_back(std::thread(std::bind(&ThreadPool::work,this))); //move?
    }
}

void ThreadPool::quit() {
    _quit = true;
    _taskpool->Quit();
}

void ThreadPool::join() {
    //bug: 不能是_threadnum,因为可能还没run
    for (int i=0; i<_threadpool.size(); ++i){
        if (_threadpool[i].joinable()){
            _threadpool[i].join();
        }
    }
}

void ThreadPool::work() {
    while (!_quit){
        spConnection conn = _taskpool->getTask();
//        assert(conn!=nullptr);
        if (conn != nullptr){
            conn->handleEvent();
        }
    }
//    std::thread::id tid = std::this_thread::get_id();
    LOG_INFO("Work Thread Quit.");
}