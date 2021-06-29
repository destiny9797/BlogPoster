#include <iostream>
#include <signal.h>
#include "HttpServer.h"

HttpServer httpserver(80, "For 0c037ea28ec644b99fd4cdf671b90f15.html");

void handler(int sig){
    httpserver.Quit();
}


int main() {
    signal(SIGINT, handler);

    httpserver.Start();

    return 0;
}
