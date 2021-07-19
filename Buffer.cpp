//
// Created by zhujiaying on 2021/6/24.
//


#include "Buffer.h"
#include <unistd.h>
#include <sys/uio.h>
#include <assert.h>
#include <errno.h>

#define RMEM 137072

Buffer::Buffer(int initlen)
    : _data(initlen),
      _readindex(0),
      _writeindex(0)
{

}

Buffer::~Buffer() {

}

void Buffer::init() {
    _data.clear();
    _readindex = 0;
    _writeindex = 0;
}

int Buffer::Read(int fd, int& err) {
    int avail_space = writableSpace();
    char buffer2[RMEM];
    struct iovec iov[2];
    iov[0].iov_base = writePtr();
    iov[0].iov_len = avail_space;
    iov[1].iov_base = buffer2;
    iov[1].iov_len = 65536;
    int nread = readv(fd,iov,2);
    if (nread < 0){
        //no more data
        err = errno;
        return -1;
    }
    else if (nread <= avail_space){
        _writeindex += nread;
    }
    else{
        _writeindex = _data.size();
        append(buffer2, nread - avail_space);
    }
    return nread;
}

int Buffer::Write(int fd, int& err) {
    int datalen = _writeindex - _readindex;
    assert(datalen>=0);
    if (datalen == 0){
        return 0;
    }
    int nwrite = write(fd, &*_data.begin()+_readindex, datalen);
    if (nwrite < 0){
        //can not write anymore
        err = errno;
        return -1;
    }
    else{
        _readindex += nwrite;
    }
    return nwrite;
}

void Buffer::append(const char *buf, int len) {
    assert(buf!=nullptr);
    if (writableSpace() < len){
        makeSpace(len);
    }
    assert(writableSpace() >= len);
    std::copy(buf, buf+len, _data.begin()+_writeindex);
    _writeindex += len;
}

void Buffer::append(const std::string &str) {
    append(str.c_str(), str.length());
}

void Buffer::makeSpace(int len) {
    if (writableSpace() + prependableSpace() < len){
        _data.resize(_writeindex + len);
    }
    else{
        std::copy(_data.begin()+_readindex, _data.begin()+_writeindex, _data.begin());
        _writeindex -= _readindex;
        _readindex = 0;
    }
}

int Buffer::writableSpace() {
    return _data.size() - _writeindex;
}

int Buffer::prependableSpace() {
    return _readindex;
}

char* Buffer::readPtr() {
    return &*_data.begin() + _readindex;
}

char* Buffer::writePtr() {
    return &*_data.begin() + _writeindex;
}

void Buffer::updateReadptr(int len) {
    assert(_readindex+len<=_writeindex);
    _readindex += len;
}

int Buffer::datasize() {
    return _writeindex - _readindex;
}