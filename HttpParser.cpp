//
// Created by zhujiaying on 2021/6/21.
//

#include "HttpParser.h"
#include <iostream>
#include <sstream>

//从状态机，解析出一行内容
LINE_STATE parseLine(char* buffer, int& checked_index, int& read_index){
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
HTTP_CODE parseRequseline(char* temp, CHECK_STATE& checkState, REQ_CODE& requestCode){
    std::string str(temp);
    std::stringstream sstream(str);
    std::string method, url, version;
    sstream >> method;
    sstream >> url;
    sstream >> version;
    if (method.empty() || url.empty() || version.empty()){
        return BAD_REQUEST;
    }
    if (method=="GET"){
        requestCode = REQ_GET;
        std::cout << "GET request" << std::endl;
    }
    else if (method=="POST"){
        requestCode = REQ_POST;
        std::cout << "POST request" << std::endl;
    }
    else{
        return BAD_REQUEST;
    }

    if (version!="HTTP/1.1" && version!="HTTP/1.0"){
        return BAD_REQUEST;
    }

    std::cout << "The request url is: " << url << std::endl;
    checkState = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

//分析头部字段
HTTP_CODE parseHeaders(char* temp){
    if (temp[0]=='\0'){
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

//入口函数
HTTP_CODE parseContent(char* buffer, int& checked_index, CHECK_STATE& checkState, int& read_index, int& start_line, REQ_CODE& reqCode){
    LINE_STATE linestatus = LINE_OK;
    HTTP_CODE retcode = NO_REQUEST;
    //只有当解析出一行了才会进行行解析
    while ((linestatus=parseLine(buffer, checked_index, read_index)) == LINE_OK){
        std::cout << "one line parsed, " << std::string(buffer+start_line, buffer+checked_index) << std::endl;
        char* temp = buffer + start_line;
        start_line = checked_index;
        switch (checkState) {
            case CHECK_STATE_REQUESTLINE:
                retcode = parseRequseline(temp, checkState, reqCode);
                if (retcode==BAD_REQUEST){
                    return BAD_REQUEST;
                }
                break;
            case CHECK_STATE_HEADER:
                retcode = parseHeaders(temp);
                if (retcode==GET_REQUEST){
                    return GET_REQUEST;
                }
                else if (retcode==BAD_REQUEST){
                    return BAD_REQUEST;
                }
                break;
            default:
                return INTERNAL_ERROR;
        }
    }
    if (linestatus==LINE_OPEN){
        return NO_REQUEST;
    }
    else{
        return BAD_REQUEST;
    }
}


