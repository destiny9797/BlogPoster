//
// Created by zhujiaying on 2021/6/21.
//

#include "HttpParser.h"

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <assert.h>

//#include <mysqlx/xdevapi.h>


HttpParser::HttpParser()
    : checkState(CHECK_STATE_REQUESTLINE),
      content_len(0),
      keepalive(false)
{

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
    keepalive = false;
}

void HttpParser::parseUrl(std::string &url) {
    if (url == "/" || url == "/For"){
        url = "/ohohoh 998069b3e639400cb81e32dbaa4fee17.html";
    }
    else{
        urldecode(&*url.begin(), &*url.begin());
    }
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

    parseUrl(url);
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
    else if (header=="Connection:" && content=="keep-alive"){
        keepalive = true;
    }
}

void HttpParser::parseBody(std::string& line) {
    body += line;
    assert(body.size() <= content_len);
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
//                url  = pathdir + url;
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

void HttpParser::urldecode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a')
                a -= 'a'-'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';
            if (b >= 'a')
                b -= 'a'-'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            *dst++ = 16*a+b;
            src+=3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

//void HttpParser::parsePost() {
//    if (url=="/login.html"){
//        int posbegin = body.find("username=") + 9;
//        int posend = body.find_first_of('&');
//        std::string username = body.substr(posbegin,posend);
//        posbegin = body.find("password=") + 9;
//        posend = body.size();
//        std::string password = body.substr(posbegin,posend);
//        spSqlconn conn = MysqlConnPool::getInstance().getSqlconn();
//        mysqlx::Schema db = conn->getSchema("webserver");
//        mysqlx::Table table = db.getTable("user");
//
//        table.select("username", "password").where("username= :username").bind("username",username).execute();
//    }
//
//}


