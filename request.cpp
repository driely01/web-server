#include "Server.hpp"

void Server::recieverequest( int const &i ) {

	std::vector<clients_t>::iterator itclients;
	itclients = this->findActiveClient( i );
	if ( itclients != clients.end() ) {
		char recievebuff[SIZE];
		int sender = pfds[i].fd;
		int recieved = recv( pfds[i].fd, recievebuff, SIZE, 0 );
		if ( recieved <= 0 ) {

			if ( recieved == 0 )
				std::cout << "*-*> pollserver: socket " << sender << " hung up" << std::endl;
		else
			perror( "recv" );
		this->removeclient( itclients );
		this->removepollsock( i );
		} else {
			recievebuff[recieved] = '\0';
			std::cout << "request: " << recievebuff << std::endl;
			parseRequest((char *)recievebuff, itclients);
			pfds[i].events = POLLOUT;
			std::cout << pfds[i].fd << " " << itclients->sockfd << " " << "----> pullout" << std::endl;
		}
	}
}

void	Server::parseRequest( char *recievebuff, std::vector<clients_t>::iterator& itclients ) {
	std::string recbuff(recievebuff);
	std::string requestLine;
	std::istringstream recbuffStream(recbuff);

	//get request line "GET / HTTP/1.1"
	std::getline(recbuffStream, requestLine);

	//parse request line
	std::istringstream methodStream(requestLine);
	std::getline(methodStream, itclients->method, ' ');
	std::getline(methodStream, itclients->path, ' ');
	itclients->path.erase(0, 1);

	if (access(itclients->path.c_str(), F_OK) == -1 && itclients->path != "") {
		itclients->statusLine = "HTTP/1.1 404 Not Found";
	}
	else {
		if (itclients->path == "")
			itclients->path = "index.html";
		itclients->file = open( itclients->path.c_str(), O_RDONLY, 0777 );
		if ( itclients->file == -1 ) {
			std::cerr << "failed to open file" << std::endl;
			exit( EXIT_FAILURE );
		}
		itclients->statusLine = "HTTP/1.1 200 OK";
		itclients->contentLength = get_size_fd(itclients->file);
	}
}