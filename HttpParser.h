//
// Created by zhujiaying on 2021/6/21.
//

#ifndef MYWEBSERVER_HTTPPARSER_H
#define MYWEBSERVER_HTTPPARSER_H

#include <string>

//主状态机的两种状态：当前正在分析请求行，当前正在分析头部字段
enum CHECK_STATE{ CHECK_STATE_REQUESTLINE=0, CHECK_STATE_HEADER, CHECK_STATE_BODY };

//从状态机的三种状态
enum LINE_STATE{ LINE_OK=0, LINE_BAD, LINE_OPEN };

//处理HTTP请求的结果
enum HTTP_CODE{ NO_REQUEST, GET_REQUEST, BAD_REQUEST, FORBIDDON_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };

enum REQ_CODE{ NO_REQ, REQ_GET, REQ_POST };

class HttpParser{
public:
    HttpParser(char* buffer, int len);

    ~HttpParser();

    //入口函数
    HTTP_CODE parseContent();

    REQ_CODE getMethod(){ return requestCode; }

    const std::string& getUrl(){ return url; }

    int getParsedlen(){ return parsed_len; }

private:
    //从状态机，解析出一行内容
    LINE_STATE parseLine(char* buffer);

    //分析请求行
    HTTP_CODE parseRequseline(char* temp);

    //分析头部字段
    HTTP_CODE parseHeaders(char* temp);

    //分析数据字段
    HTTP_CODE parseBody(char* temp);

private:
    char* buffer;

    std::string url;

    std::string version;

    int checked_index;

    int read_index;

    int start_line;

    CHECK_STATE checkState;

    REQ_CODE requestCode;

    std::string path;

    int parsed_len;

    int content_len;

    std::string content;

};






#endif //MYWEBSERVER_HTTPPARSER_H
