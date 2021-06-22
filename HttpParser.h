//
// Created by zhujiaying on 2021/6/21.
//

#ifndef MYWEBSERVER_HTTPPARSER_H
#define MYWEBSERVER_HTTPPARSER_H

    //主状态机的两种状态：当前正在分析请求行，当前正在分析头部字段
    enum CHECK_STATE{ CHECK_STATE_REQUESTLINE=0, CHECK_STATE_HEADER };

    //从状态机的三种状态
    enum LINE_STATE{ LINE_OK=0, LINE_BAD, LINE_OPEN };

    //处理HTTP请求的结果
    enum HTTP_CODE{ NO_REQUEST, GET_REQUEST, BAD_REQUEST, FORBIDDON_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };

    enum REQ_CODE{ REQ_GET, REQ_POST };

    //入口函数
    HTTP_CODE parseContent(char* buffer, int& checked_index, CHECK_STATE& checkState, int& read_index, int& start_line, REQ_CODE& reqCode);

    //从状态机，解析出一行内容
    LINE_STATE parseLine(char* buffer, int& checked_index, int& read_index);

    //分析请求行
    HTTP_CODE parseRequseline(char* temp, CHECK_STATE& checkState, REQ_CODE& requestCode);

    //分析头部字段
    HTTP_CODE parseHeaders(char* temp);




#endif //MYWEBSERVER_HTTPPARSER_H
