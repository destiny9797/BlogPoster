//
// Created by zhujiaying on 2021/6/19.
//

#include "HttpServer.h"
#include "TcpServer.h"
#include "TaskPool.h"
#include "HttpConnection.h"
#include "HttpParser.h"

#include <string.h>
#include <iostream>
#include <functional>


HttpServer::HttpServer(int port)
    : _taskpool(std::make_shared<TaskPool>()),
      _tcpserver(std::make_shared<TcpServer>(port, _taskpool))
{
    _tcpserver->setParseRequest(std::bind(&HttpServer::ParseRequest, this, std::placeholders::_1, std::placeholders::_2));
}

HttpServer::~HttpServer() {
    std::cout << "~HttpServer()" << std::endl;
}


HTTP_CODE HttpServer::ParseRequest(std::string &msg, std::string &msg_response) {
    int msglen = msg.length();
    char buffer[msglen+1];
    strcpy(buffer, msg.c_str());
    //只要收到的不完整，下次就重新解析
    int checked_index = 0, read_index = msglen, start_line = 0;
    CHECK_STATE checkstate = CHECK_STATE_REQUESTLINE;
    REQ_CODE reqCode = REQ_GET;
    HTTP_CODE httpcode = parseContent(buffer, checked_index, checkstate, read_index, start_line, reqCode);

    if (httpcode == GET_REQUEST){
        std::string msgbody(2000000, 'w');
        if (reqCode == REQ_GET){
            msg_response = "HTTP/1.0 200 OK\r\n";
            msg_response += "Server: ZhuJiaying's webserver\r\n";
            msg_response += "Content-Type: text/html; charset=utf-8\r\n";
            msg_response += "Content-Length: " + std::to_string(msgbody.length()) + "\r\n";
            msg_response += "\r\n";
            msg_response += msgbody;
        }
    }
//    std::cout << "HTTP_CODE=" << httpcode << ", msg_response=" << msg_response << std::endl;

    return httpcode;
}


void HttpServer::Start() {
//    _threadpool.run();
    _tcpserver->Start();
}

void HttpServer::Quit() {
    _tcpserver->Quit();
//    _threadpool.quit();
}
