#include "HttpServer.h"
#include <signal.h>


int main() {
    signal(SIGINT, [](int){HttpServer::getInstance().Quit();});


    HttpServer& httpserver = HttpServer::getInstance();
    httpserver.Init(80, "藏宝阁 2d05c1d04c964b92852be60208ad61d3.html");

    httpserver.Start();

    return 0;
}
