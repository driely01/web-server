#pragma once

#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>

#define PORT "9034"
#define BACKLOG 128
#define SEND 1024
#define SIZE 1024

typedef struct clients_s {

	int sockfd;
	size_t content;
	size_t contentResponse;
	size_t contentLength;
	int file;
	std::string method;
	std::string path;
	std::string message;
	std::string statusLine;
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
		void	response( std::string path, std::vector<clients_t>::iterator& itclient );
		void	recieverequest( int const &i );
		void	sendresponse( int const &i );

		//request
		void	parseRequest( char *recievebuff, std::vector<clients_t>::iterator& itclients );
};

std::string	getMimeType(const std::string& extension);
size_t		get_size_fd(int fd);