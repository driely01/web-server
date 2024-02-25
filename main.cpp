#include "Server.hpp"

int main( void ) {

    Server server;

    signal(SIGPIPE, SIG_IGN);
    server.getInfoAddr();
    server.socketlistener();
    server.response();
    while ( 1 ) {

        server.pollMainWork();
    }
    return 0;
}