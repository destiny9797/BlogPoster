#include <iostream>
#include <signal.h>
#include "HttpServer.h"

HttpServer httpserver(80);

void handler(int sig){
    httpserver.Quit();
}


int main() {
    signal(SIGINT, handler);

    httpserver.Start();

    return 0;
}
