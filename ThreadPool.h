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
class Connection;

class ThreadPool{
public:
    typedef std::shared_ptr<TaskPool> spTaskPool;
    typedef std::shared_ptr<Connection> spConnection;

    ThreadPool(int threadnum, spTaskPool taskpool);

    ~ThreadPool();

    void run();

    void work();

    void quit();

    void join();


private:
    int _threadnum;

    bool _quit;

    std::vector<std::thread> _threadpool;

    spTaskPool _taskpool;


};

#endif //MYWEBSERVER_THREADPOOL_H
