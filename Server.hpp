#pragma once

#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/fcntl.h>
#include <vector>
#include <errno.h>
#include <algorithm>
#include <fstream>
#include <sstream>

#define PORT "9034"
#define BACKLOG 128
#define SEND 1024
#define SIZE 1024

typedef struct clients_s {

    int sockfd;
    size_t content;
} clients_t;

class Server {

    private:
        int listener;
        int yes;
        int status;
        struct addrinfo hints;
        struct addrinfo *ai;
        struct addrinfo *p;

        // poll and accept
        struct sockaddr_storage remoteaddr;
        socklen_t addrlen;
        char remoteIp[INET6_ADDRSTRLEN]; // this var just for debugging
        std::vector<struct pollfd> pfds;

        // clients
        std::vector<clients_t> clients;

        // response
        std::string message;

        // some fuction that we don't need to use outside the class
        int createsocket( void );


    public:
        Server( void );
        ~Server( void );

        // this fuctions just for debugging
        void *getinaddr( struct sockaddr *sa ); // debug
        void printConeectedaddr( int const &sockfd );  // debug

        // getaddrinfo create, bind and listen the socket
        void getInfoAddr( void );
        void socketlistener( void );

        // poll and accept connections
        void addServersToPoll( void );
        void pollMainWork( void );
        void addPollClient( int const &sockfd );
        int acceptConnections( void );

        void removepollsock( int const &index );


        // clients functions
        void addClient( int const &sockfd );
        void removeclient( std::vector<clients_t>::iterator const &it );
        std::vector<clients_t>::iterator findActiveClient( int const &i );

        // ---------------------------- test ---------------------------- //
        std::string response( std::string path );
        void recieverequest( int const &i );
        void sendresponse( int const &i );

        //request
        void	parseRequest( char *recievebuff );


};

//mime type
std::string getMimeType(const std::string& extension);