//
// Created by zhujiaying on 2021/6/24.
//

#include "HttpResponse.h"

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
      _path()
{

}

HttpResponse::~HttpResponse() {

}

void HttpResponse::setStatus(int code, const std::string &path) {
    _code = code;
    _path = path;
}
void HttpResponse::setResponse(Buffer &buffer) {
    buffer.append("HTTP/1.1 200 OK\r\n");
    buffer.append("Server: ZhuJiaying's webserver\r\n");
    buffer.append("Content-Type: text/html; charset=utf-8\r\n");
    buffer.append("Content-Length: 13\r\n");// + std::to_string(filecontent.length()) + "\r\n");
    buffer.append("\r\n");
    buffer.append("Hello, world.");

}