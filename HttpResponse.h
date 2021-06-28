//
// Created by zhujiaying on 2021/6/24.
//

#ifndef MYWEBSERVER_HTTPRESPONSE_H
#define MYWEBSERVER_HTTPRESPONSE_H

#include "Buffer.h"
#include <string>
#include <unordered_map>
#include <sys/stat.h>

class HttpResponse{
public:
    HttpResponse();

    ~HttpResponse();

    void init();

    void setStatus(const std::string& method, const std::string &url);

    void setResponse(Buffer& buffer);

    int getFile(){ return _filefd; }

    int getFileSize(){ return _fileinfo.st_size; }

    int getOffset(){ return _offset; }

    void setOffset(off_t offset){ _offset = offset; }

private:
    void addResponseLine(Buffer& buffer);

    void addHeader(Buffer& buffer);

    void addBody(Buffer& buffer);

    bool getResourse(const std::string& url);

    int _code;

    std::string _method;

    std::string _url;

    std::string _pathdir;

    struct stat _fileinfo;

    int _filefd;

    off_t _offset;
};

#endif //MYWEBSERVER_HTTPRESPONSE_H
