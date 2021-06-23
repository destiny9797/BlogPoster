//
// Created by zhujiaying on 2021/6/19.
//

#include "HttpServer.h"
#include "TcpServer.h"
#include "TaskPool.h"
#include "Connection.h"
#include "HttpParser.h"

#include <string.h>
#include <iostream>
#include <functional>
#include <fstream>
#include <sstream>


HttpServer::HttpServer(int port)
    : _taskpool(std::make_shared<TaskPool>()),
      _tcpserver(std::make_shared<TcpServer>(port, _taskpool))
{
    _tcpserver->setParseRequest(std::bind(&HttpServer::ParseRequest, this, std::placeholders::_1, std::placeholders::_2));
}

HttpServer::~HttpServer() {
    std::cout << "~HttpServer()" << std::endl;
}


int HttpServer::ParseRequest(std::string &msg, std::string &msg_response) {
    int msglen = msg.length();
    char buffer[msglen+1];
    strcpy(buffer, msg.c_str());
    //只要收到的不完整，下次就重新解析
//    int checked_index = 0, read_index = msglen, start_line = 0;
//    CHECK_STATE checkstate = CHECK_STATE_REQUESTLINE;
//    REQ_CODE reqCode = REQ_GET;

    int parsed_len = 0;
    HTTP_CODE httpcode = NO_REQUEST;
    do{
        HttpParser parser(buffer+parsed_len, msglen-parsed_len);
        httpcode = parser.parseContent();
        parsed_len += parser.getParsedlen();
//    HTTP_CODE httpcode = parseContent(buffer, checked_index, checkstate, read_index, start_line, reqCode, url);

        if (httpcode == GET_REQUEST){
//        std::string msgbody("Hello, world.");
            REQ_CODE method = parser.getMethod();
            const std::string& url = parser.getUrl();
            std::cout << "url=" << url << std::endl;
            if (method == REQ_GET){
                std::string filecontent;
                std::ifstream file;
                file.open(url.c_str(), std::ios::in);
                if (!file){
                    std::cout << "No such file!" << std::endl;
                    filecontent = "No such file!";
                }
                else{
                    std::stringstream readbuf;
                    readbuf << file.rdbuf();
                    filecontent = readbuf.str();
                }

                msg_response = "HTTP/1.0 200 OK\r\n";
                msg_response += "Server: ZhuJiaying's webserver\r\n";
                msg_response += "Content-Type: text/html; charset=utf-8\r\n";
                msg_response += "Content-Length: " + std::to_string(filecontent.length()) + "\r\n";
                msg_response += "\r\n";
                msg_response += filecontent;
            }
            else if (method == REQ_POST){

            }
        }
    }while (httpcode==GET_REQUEST && parsed_len<msglen);

//    std::cout << "HTTP_CODE=" << httpcode << ", msg_response=" << msg_response << std::endl;

    return parsed_len;
}


void HttpServer::Start() {
//    _threadpool.run();
    _tcpserver->Start();
}

void HttpServer::Quit() {
    _tcpserver->Quit();
//    _threadpool.quit();
}
