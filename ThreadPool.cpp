//
// Created by zhujiaying on 2021/6/19.
//

#include "ThreadPool.h"
#include "TaskPool.h"
#include "HttpConnection.h"
#include <sys/epoll.h>
#include <iostream>

ThreadPool::ThreadPool(int threadnum, spTaskPool taskpool)
    : _threadnum(threadnum), _taskpool(taskpool), _quit(false){

}

ThreadPool::~ThreadPool() {
    std::cout << "~ThreadPool()" << std::endl;
}

void ThreadPool::run() {
    for (int i=0; i<_threadnum; ++i){
        _threadpool.push_back(std::thread(std::bind(&ThreadPool::work,this))); //move?
    }
}

void ThreadPool::quit() {
    _quit = true;
}

void ThreadPool::join() {
    for (int i=0; i<_threadnum; ++i){
        if (_threadpool[i].joinable()){
            _threadpool[i].join();
        }
    }
}

void ThreadPool::work() {
    while (!_quit){
        spHttpConnection conn = _taskpool->getTask();
        if (conn != nullptr){
            uint32_t event = conn->getEvent();
            if (event & (EPOLLIN | EPOLLPRI)){ //对方有数据或正常关闭
                std::cout << "one thread handle read" << std::endl;
                handleRead(conn);
            }
            else if (event & EPOLLOUT){
                std::cout << "one thread handle write" << std::endl;
                handleWrite(conn);
            }
            else if (event & (EPOLLERR | EPOLLHUP)){
                std::cout << "one thread handle error" << std::endl;
                handleError(conn);
            }
        }
    }
}