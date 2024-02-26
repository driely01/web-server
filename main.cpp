#include "Server.hpp"

int main( void ) {

    Server server;

    signal(SIGPIPE, SIG_IGN);
    server.getInfoAddr();
    server.socketlistener();
    while ( 1 ) {

        server.pollMainWork();
    }
    return 0;
}