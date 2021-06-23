//
// Created by zhujiaying on 2021/6/23.
//

#ifndef MYWEBSERVER_TIMERMANAGER_H
#define MYWEBSERVER_TIMERMANAGER_H

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <functional>

class Timer;

class TimerManager{
public:
    typedef std::shared_ptr<Timer> spTimer;
    typedef std::function<void(int)> callback;

    TimerManager(int maxfd, const callback& handle);

    ~TimerManager();

    void addTimer(int fd, int timeout);

    void updateTimer(int fd, int timeout);

    void rmTimer(int fd);


    void closeExpire();

private:

    std::mutex _mutex;

    std::vector<spTimer> _timers;


    callback handleExpire;


};

#endif //MYWEBSERVER_TIMERMANAGER_H
