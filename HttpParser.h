//
// Created by zhujiaying on 2021/6/21.
//

#ifndef MYWEBSERVER_HTTPPARSER_H
#define MYWEBSERVER_HTTPPARSER_H

#include "Buffer.h"
#include <string>
#include <unordered_map>

//主状态机的两种状态：当前正在分析请求行，当前正在分析头部字段
enum CHECK_STATE{ CHECK_STATE_REQUESTLINE=0, CHECK_STATE_HEADER, CHECK_STATE_BODY, FINISH };

//从状态机的三种状态
//enum LINE_STATE{ LINE_OK=0, LINE_BAD, LINE_OPEN };

//处理HTTP请求的结果
enum HTTP_CODE{ NO_REQUEST, GET_REQUEST, BAD_REQUEST, INTERNAL_ERROR};

//enum REQ_CODE{ NO_REQ, REQ_GET, REQ_POST };

class HttpParser{
public:
    HttpParser();

    ~HttpParser();

    void init();

    //入口函数
    HTTP_CODE parseContent(Buffer& buffer);

    const std::string& getMethod(){ return method; }

    const std::string& getUrl(){ return url; }

    bool isKeepalive(){ return keepalive; }


private:
    //从状态机，解析出一行内容
//    LINE_STATE parseLine(char* buffer);

    //分析请求行
    bool parseRequseline(std::string& line);

    //分析头部字段
    void parseHeaders(std::string& line);

    //分析数据字段
    void parseBody(std::string& line);

private:

    std::string method;

    std::string url;

    std::string version;

    std::unordered_map<std::string, std::string> headers;

    std::string body;

    CHECK_STATE checkState;

//    std::string pathdir;

    int content_len;

    bool keepalive;

};






#endif //MYWEBSERVER_HTTPPARSER_H
