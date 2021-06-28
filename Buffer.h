//
// Created by zhujiaying on 2021/6/24.
//

#ifndef MYWEBSERVER_BUFFER_H
#define MYWEBSERVER_BUFFER_H

#include <vector>
#include <string>
#include <iostream>
#include <assert.h>

class Buffer{
public:
    Buffer(int initlen);

    ~Buffer();

    int Read(int fd, int& err);

    int Write(int fd, int& err);

    char* readPtr();

    char* writePtr();

    void init();

    void append(const std::string& str);

    void updateReadptr(int len);

    int datasize();

    //调试用
    void printdata(){
        assert(_readindex<=_writeindex);
        std::string msg(readPtr(),writePtr());
        std::cout << msg << std::endl;
    }

private:
    void append(const char* buf, int len);

    void makeSpace(int len);

    int prependableSpace();

    int writableSpace();

    std::vector<char> _data;

    int _readindex;

    int _writeindex;

};

#endif //MYWEBSERVER_BUFFER_H
