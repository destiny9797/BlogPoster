//
// Created by zhujiaying on 2021/6/25.
//

#include "Log.h"
#include <sys/time.h>
#include <assert.h>
#include <cstring>
#include <iostream>

#define MAX_BUFSIZE 256

using namespace std;

Log & Log::getInstance() {
    static Log instance;
    return instance;
}

Log::Log()
    : _open(false), _quit(false), _fs()
{
}

Log::~Log() {
    // bug: 要调用quit()，以防服务器打开失败
    Quit();
//    std::cout << "~Log()" << std::endl;
}

void Log::Init(const std::string &path, int maxQueueLen) {
    _path = path;
    _queuelen = maxQueueLen;
    _async = _queuelen>0;
    _asyncThr = nullptr;
}

void Log::Start() {

    time_t timer = time(nullptr);
    struct tm* tt = localtime(&timer);
    struct tm t = *tt;
    char filename[200];
    sprintf(filename, "%s/%04d_%02d_%02d_%02d_%02d_%02d.log", _path.c_str(), t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

    _fs.open(filename, std::ios::out | std::ios::app);
    assert(_fs);

    if (_queuelen > 0){
        _asyncThr = std::make_unique<std::thread>(&Log::asyncWork, this);
//        _asyncThr->detach();
    }

    _open = true;

}

void Log::Quit() {
    _open = false;
    _quit = true;
    _cond.notify_one();

    if (_asyncThr!=nullptr && _asyncThr->joinable()){
        _asyncThr->join();
    }
}


void Log::write(int level, const char* format, ...) {
    //做出要写入日志的buf
    char buf[MAX_BUFSIZE];
    int writeptr = 0;
    va_list valist;

    struct timeval tv;
    gettimeofday(&tv,nullptr);
    time_t timer = tv.tv_sec;
    struct tm* tt = localtime(&timer);
    struct tm t = *tt;
    snprintf(buf+writeptr, MAX_BUFSIZE-writeptr, "%04d-%02d-%02d %02d:%02d:%02d.%06ld ",
             t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec);
    writeptr += 27;

    strncpy(buf+writeptr, _levelmap[level].c_str(), 9);
    writeptr += 9;


    va_start(valist, format);
    int m = vsnprintf(buf+writeptr, MAX_BUFSIZE-writeptr, format, valist);
    writeptr += m;
    va_end(valist);

//    buf[writeptr++] = '\n';
    buf[writeptr++] = '\0';
    std::string bufstr(buf);
//    cout << buf << endl;

    //lock
    if (_async){
        std::unique_lock<std::mutex> lk(_mutex);
        _queue.push(bufstr);
        _cond.notify_one();
    }
    else{
        std::unique_lock<std::mutex> lk(_mutex);
        _fs << bufstr;
    }

}

void Log::asyncWork() {
    while (!_quit || hasLog()){
        std::unique_lock<std::mutex> lk(_mutex);
        if (!_queue.empty()){
            _fs << _queue.front() << endl;
            _queue.pop();
        }
        else{
            _cond.wait(lk);
        }
    }
    _fs << "Log Thread Quit" << endl;
    _fs.close();
}

bool Log::hasLog() {
    std::unique_lock<std::mutex> lk(_mutex);
    return !_queue.empty();
}