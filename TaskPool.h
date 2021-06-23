//
// Created by zhujiaying on 2021/6/19.
//

#ifndef MYWEBSERVER_TASKPOOL_H
#define MYWEBSERVER_TASKPOOL_H

#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class Connection;

class TaskPool{
public:
//    typedef std::function<void()> task;
    typedef std::shared_ptr<Connection> spHttpConnection;

    TaskPool(); //singleton

    ~TaskPool();

    void addTask(spHttpConnection conn);

    spHttpConnection getTask();

    void notifyAll() { _cond.notify_all(); }

private:
    std::queue<spHttpConnection> _activeconn;

    std::mutex _mutex;

    std::condition_variable _cond;

};

#endif //MYWEBSERVER_TASKPOOL_H
