//
// Created by zhujiaying on 2021/6/24.
//

#ifndef MYWEBSERVER_HTTPRESPONSE_H
#define MYWEBSERVER_HTTPRESPONSE_H

#include "Buffer.h"
#include <string>
#include <unordered_map>

class HttpResponse{
public:
    HttpResponse();

    ~HttpResponse();

    void setStatus(int code, const std::string &path);

    void setResponse(Buffer& buffer);

private:
    int _code;

    std::string _path;
};

#endif //MYWEBSERVER_HTTPRESPONSE_H
