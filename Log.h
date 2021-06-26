//
// Created by zhujiaying on 2021/6/25.
//

#ifndef MYWEBSERVER_LOG_H
#define MYWEBSERVER_LOG_H

#include <string>
#include <queue>
#include <thread>
#include <memory>
#include <fstream>
#include <stdarg.h>
#include <mutex>
#include <condition_variable>

#define LEVEL_NUM 4

//单例模式
class Log{
public:
    //同步：maxQueueLen=0；异步：maxQueueLen>0
    static Log& getInstance();

    void Init(const std::string& path, int maxQueueLen = 0);

    void Start();

    void Quit();

    bool isOpen(){ return _open; }

    void write(int level, const char* format, ...);

private:
    Log();

    ~Log();

    Log(const Log& log){}

    Log& operator=(Log& log){}

    void asyncWork();

    bool hasLog();


    const std::string _levelmap[LEVEL_NUM] = {
            "[Info] : ",
            "[Debug]: ",
            "[Warn] : ",
            "[Error]: "
    };



    std::string _path;

    int _queuelen;

    bool _async;

    std::queue<std::string> _queue;

    std::unique_ptr<std::thread> _asyncThr;

    std::mutex _mutex;

    std::condition_variable _cond;

    std::ofstream _fs;

    bool _open;

    bool _quit;

};

#define LOG_BASE(level, format, ...)   \
    do {                               \
        Log& log = Log::getInstance(); \
        if (log.isOpen()){             \
            log.write(level, format, ##__VA_ARGS__);  \
        }                              \
    } while(0);


#define LOG_INFO(format, ...)  do{LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_DEBUG(format, ...) do{LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...)  do{LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do{LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //MYWEBSERVER_LOG_H
