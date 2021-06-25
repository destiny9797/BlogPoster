//
// Created by zhujiaying on 2021/6/21.
//

#include "HttpParser.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <assert.h>

HttpParser::HttpParser()
    : checkState(CHECK_STATE_REQUESTLINE),
      content_len(0)
{
    char* curpath = getcwd(NULL, 0);
    if (curpath==nullptr){
        perror("getcwd wrong:");
    }
    else{
        path = std::string(curpath) + "/../data";
    }
}

HttpParser::~HttpParser() {
//    std::cout << "~HttpParser()" << std::endl;
}

void HttpParser::init() {
    checkState = CHECK_STATE_REQUESTLINE;
    content_len = 0;
    body = "";
    method = "";
    url = "";
    version = "";
    headers.clear();
}


//分析请求行
bool HttpParser::parseRequseline(std::string& line){
    std::stringstream sstream(line);
    sstream >> method;
    sstream >> url;
    sstream >> version;
    if (method.empty() || url.empty() || version.empty()){
        return false;
    }

    checkState = CHECK_STATE_HEADER;
    return true;
}

//分析头部字段
void HttpParser::parseHeaders(std::string& line){
    if (line.empty()){
        checkState = content_len==0 ? FINISH : CHECK_STATE_BODY;
        return;
    }
    std::stringstream sstream(line);
    std::string header, content;
    sstream >> header;
    sstream >> content;
    headers[header] = content;

    if (header=="Content-Length:"){
        content_len = std::stoi(content);
    }
}

void HttpParser::parseBody(std::string& line) {
    body += line;
    assert(body.size() < content_len);
    if (body.size()==content_len){
        checkState = FINISH;
    }
};

//入口函数
HTTP_CODE HttpParser::parseContent(Buffer& buffer){
    //只有当解析出一行了才会进行行解析
    char crlf[] = "\r\n";
    while (buffer.datasize() > 0 && checkState!=FINISH){
        char* lineEnd = std::search(buffer.readPtr(), buffer.writePtr(), crlf, crlf+2);
        if (lineEnd==buffer.writePtr()){
            break;
        }
        std::string line(buffer.readPtr(), lineEnd);
//        std::cout << "oneline*** " << line << std::endl;
//        std::cout << "state: " << checkState << std::endl;
        switch (checkState) {
            case CHECK_STATE_REQUESTLINE:
                if (!parseRequseline(line)){
                    return NO_REQUEST;
                }
                url  = path + url;
                break;
            case CHECK_STATE_HEADER:
                parseHeaders(line);
                break;
            case CHECK_STATE_BODY:
                parseBody(line);
                break;
            default:
                return INTERNAL_ERROR;
        }
        buffer.updateReadptr(lineEnd-buffer.readPtr()+2);
    }

    if (checkState==FINISH)
        return GET_REQUEST;
    else
        return NO_REQUEST;


}


