//
// Created by zhujiaying on 2021/6/21.
//

#include "HttpParser.h"
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <algorithm>

HttpParser::HttpParser(char* buffer, int len)
    : buffer(buffer),
      checked_index(0),
      read_index(len),
      start_line(0),
      checkState(CHECK_STATE_REQUESTLINE),
      requestCode(NO_REQ),
      parsed_len(0),
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
    std::cout << "~HttpParser()" << std::endl;
}

//从状态机，解析出一行内容
LINE_STATE HttpParser::parseLine(char* buffer){
    for (; checked_index<read_index; ++checked_index){
        char temp = buffer[checked_index];
        if (temp=='\r'){
            if (checked_index+1==read_index){
                return LINE_OPEN;
            }
            else if (buffer[checked_index+1]=='\n'){
                buffer[checked_index++] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if (temp=='\n'){
            if (checked_index>1 && buffer[checked_index-1]=='\r'){
                buffer[checked_index-1] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

//分析请求行
HTTP_CODE HttpParser::parseRequseline(char* temp){
    std::string str(temp);
    std::stringstream sstream(str);
    std::string method;
    sstream >> method;
    sstream >> url;
    sstream >> version;
    if (method.empty() || url.empty() || version.empty()){
        return BAD_REQUEST;
    }
    if (method=="GET"){
        requestCode = REQ_GET;
        url = path + url;
//        std::cout << "GET request" << std::endl;
    }
    else if (method=="POST"){
        requestCode = REQ_POST;
//        std::cout << "POST request" << std::endl;
    }
    else{
        return BAD_REQUEST;
    }

    if (version!="HTTP/1.1" && version!="HTTP/1.0"){
        return BAD_REQUEST;
    }

    checkState = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

//分析头部字段
HTTP_CODE HttpParser::parseHeaders(char* temp){

    if (temp[0]=='\0'){
        if (content_len==0){
            checkState = CHECK_STATE_REQUESTLINE;
            return GET_REQUEST;
        }
        else{
            checkState = CHECK_STATE_BODY;
            return NO_REQUEST;
        }
    }

    std::string str(temp);
    std::stringstream sstream(str);
    std::string first;
    sstream >> first;
    std::transform(first.begin(),first.end(),first.begin(),::tolower);
    if (first=="content-length:"){
        std::string length;
        sstream >> length;
        content_len = std::stoi(length);
    }

    return NO_REQUEST;
}

HTTP_CODE HttpParser::parseBody(char *temp) {
    content = std::string(temp);
    if (content.length() == content_len){
        checked_index += content_len;
        checkState = CHECK_STATE_REQUESTLINE;
        return GET_REQUEST;
    }
    else if (content.length() < content_len){
        return NO_REQUEST;
    }
    else{
        content = std::string(temp, temp+content_len);
        checked_index += content_len;
        checkState = CHECK_STATE_REQUESTLINE;
        return GET_REQUEST;
    }
};

//入口函数
HTTP_CODE HttpParser::parseContent(){
    LINE_STATE linestatus = LINE_OK;
    HTTP_CODE retcode = NO_REQUEST;
    //只有当解析出一行了才会进行行解析
    while ((linestatus=parseLine(buffer)) == LINE_OK){
//        std::cout << "startline=" << start_line << ", checked=" << checked_index << ", total=" << read_index << std::endl;
//        std::cout << "parsed, *** " << std::string(buffer+start_line, buffer+checked_index) << std::endl;
        char* temp = buffer + start_line;
        switch (checkState) {
            case CHECK_STATE_REQUESTLINE:
                retcode = parseRequseline(temp);
                if (retcode==BAD_REQUEST){
                    return BAD_REQUEST;
                }
                break;
            case CHECK_STATE_HEADER:
                retcode = parseHeaders(temp);
                if (retcode==GET_REQUEST){
                    parsed_len = checked_index;
                    return GET_REQUEST;
                }
                else if (retcode==BAD_REQUEST){
                    return BAD_REQUEST;
                }
                break;
            case CHECK_STATE_BODY:
                retcode = parseBody(temp);
                if (retcode==GET_REQUEST){
                    parsed_len = checked_index;
                    return GET_REQUEST;
                }
                break;
            default:
                return INTERNAL_ERROR;
        }
        start_line = checked_index;
    }
    if (linestatus==LINE_OPEN){
        return NO_REQUEST;
    }
    else{
        return BAD_REQUEST;
    }
}


