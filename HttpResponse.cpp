//
// Created by zhujiaying on 2021/6/24.
//

#include "HttpResponse.h"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

//const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
//        { ".html",  "text/html" },
//        { ".xml",   "text/xml" },
//        { ".xhtml", "application/xhtml+xml" },
//        { ".txt",   "text/plain" },
//        { ".rtf",   "application/rtf" },
//        { ".pdf",   "application/pdf" },
//        { ".word",  "application/nsword" },
//        { ".png",   "image/png" },
//        { ".gif",   "image/gif" },
//        { ".jpg",   "image/jpeg" },
//        { ".jpeg",  "image/jpeg" },
//        { ".au",    "audio/basic" },
//        { ".mpeg",  "video/mpeg" },
//        { ".mpg",   "video/mpeg" },
//        { ".avi",   "video/x-msvideo" },
//        { ".gz",    "application/x-gzip" },
//        { ".tar",   "application/x-tar" },
//        { ".css",   "text/css "},
//        { ".js",    "text/javascript "},
//};
//
//const unordered_map<int, string> HttpResponse::CODE_STATUS = {
//        { 200, "OK" },
//        { 400, "Bad Request" },
//        { 403, "Forbidden" },
//        { 404, "Not Found" },
//};
//
//const unordered_map<int, string> HttpResponse::CODE_PATH = {
//        { 400, "/400.html" },
//        { 403, "/403.html" },
//        { 404, "/404.html" },
//};


HttpResponse::HttpResponse()
    : _code(200),
      _filefd(0),
      _offset(0)
{
    char* curpath = getcwd(NULL, 0);
    if (curpath==nullptr){
        perror("getcwd wrong:");
    }
    else{
        _pathdir = std::string(curpath) + "/../resourses";
    }

}

HttpResponse::~HttpResponse() {
//    if (_filefd > 4){
//        close(_filefd);
//    }

}

void HttpResponse::init() {
    //防止重复关闭
    if (_filefd>2){
        if (close(_filefd)<0){
            perror("close file wrong");
        }
    }

    _code = 200;
//    _method = "";
    _url = "";
    std::memset(&_fileinfo, 0, sizeof(_fileinfo));
    _filefd = 0;
    _offset = 0;
}

void HttpResponse::setStatus(int code, const std::string &url) {
    _code = code;
    _url = url;
}
void HttpResponse::setResponse(Buffer &buffer) {
    addResponseLine(buffer);

    addHeader(buffer);

    //测试时不打开文件
//    std::string filepath = _pathdir + _url;
//    _filefd = open(filepath.c_str(), O_RDONLY);
//    if (_filefd<=0){
//        perror("open file wrong:");
//    }
//    std::cout << "filepath=" << filepath.c_str() << std::endl << "_filefd=" << _filefd << std::endl;
}

void HttpResponse::addResponseLine(Buffer &buffer) {
    //测试时不打开文件
//    if (getResourse(_url)){
        _code = 200;
        buffer.append("HTTP/1.1 200 OK\r\n");
//    }
//    else{
//        _code = 404;
//        _url = "/errorpage/404.html";
//        if (!getResourse(_url)){
//            assert(S_ISREG(_fileinfo.st_mode));
//        }
//        buffer.append("HTTP/1.1 404 NOT_FOUND\r\n");
//    }
}

void HttpResponse::addHeader(Buffer &buffer) {
    buffer.append("Server: ZhuJiaying's webserver\r\n");
    buffer.append("Content-Type: text/html; charset=utf-8\r\n");
    buffer.append("Content-Length: 0\r\n");// + std::to_string(_fileinfo.st_size) + "\r\n");
    buffer.append("\r\n");
}

void HttpResponse::addBody(Buffer &buffer) {
    buffer.append("Pressure test");
}

bool HttpResponse::getResourse(const std::string& url) {
    std::string filepath = _pathdir + _url;

//    std::cout << "stat=" << stat(filepath.c_str(), &_fileinfo) << " ,mode=" << _fileinfo.st_mode << std::endl;
    return stat(filepath.c_str(), &_fileinfo)==0 && S_ISREG(_fileinfo.st_mode);
}