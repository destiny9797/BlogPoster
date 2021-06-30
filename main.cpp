#include "HttpServer.h"
#include <signal.h>


int main() {
    signal(SIGINT, [](int){HttpServer::getInstance().Quit();});


    HttpServer& httpserver = HttpServer::getInstance();
    httpserver.Init(80, "For 0c037ea28ec644b99fd4cdf671b90f15.html");

    httpserver.Start();

    return 0;
}
