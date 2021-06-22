//
// Created by zhujiaying on 2021/6/19.
//

#ifndef MYWEBSERVER_THREADPOOL_H
#define MYWEBSERVER_THREADPOOL_H


#include <vector>
#include <thread>
#include <memory>
#include <functional>

class TaskPool;
class HttpConnection;

class ThreadPool{
public:
    typedef std::shared_ptr<TaskPool> spTaskPool;
    typedef std::shared_ptr<HttpConnection> spHttpConnection;
    typedef std::function<void(spHttpConnection)> callback;

    ThreadPool(int threadnum, spTaskPool taskpool);

    ~ThreadPool();

    void run();

    void work();

    void quit();

    void join();

    void setHandleRead(const callback& cb){
        handleRead = cb;
    }

    void setHandleWrite(const callback& cb){
        handleWrite = cb;
    }

    void setHandleError(const callback& cb){
        handleError = cb;
    }

private:
    int _threadnum;

    bool _quit;

    std::vector<std::thread> _threadpool;

    spTaskPool _taskpool;


    callback handleRead;

    callback handleWrite;

    callback handleError;

};

#endif //MYWEBSERVER_THREADPOOL_H
