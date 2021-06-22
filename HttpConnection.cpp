//
// Created by zhujiaying on 2021/6/21.
//

#include "HttpConnection.h"


HttpConnection::HttpConnection(int fd) : _fd(fd), _halfclosed(false) {

}

HttpConnection::~HttpConnection() {

}

