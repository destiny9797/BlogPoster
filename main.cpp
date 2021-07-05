#include "HttpServer.h"
#include <signal.h>


int main() {
    signal(SIGINT, [](int){HttpServer::getInstance().Quit();});


    HttpServer& httpserver = HttpServer::getInstance();
    httpserver.Init(80, "藏宝阁.html");

    httpserver.Start();

    return 0;
}
