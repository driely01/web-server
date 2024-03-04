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
			std::cout << recievebuff << std::endl;
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
	if (itclients->path == "")
		itclients->path = "index.html";

	itclients->contentLength = get_size_fd(itclients->path);

	//get Range
	std::string range;
	while (std::getline(recbuffStream, range)) {
		if (range.substr(0, 13) == "Range: bytes=") {
			int numS = range.find('-') - (range.find('=') + 1);
			itclients->rangeStart = range.substr(range.find('=') + 1, numS);
			itclients->rangeStartNum = stringToInt(itclients->rangeStart);
			if (range.find('-') + 1 < range.length() - 1 )
				itclients->rangeEnd = range.substr(range.find('-') + 1);
			if (itclients->rangeEnd.empty()) {
				itclients->rangeEndNum = get_size_fd(itclients->path) - 1;
				itclients->rangeEnd = intToString(itclients->rangeEndNum);
			}
			else
				itclients->rangeEndNum = stringToInt(itclients->rangeEnd);
			break;
		}
	}
}