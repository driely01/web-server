#include "Server.hpp"

void	Server::parseRequest( char *recievebuff ) {
	std::string recbuff(recievebuff);
	std::string requestLine;
	std::istringstream recbuffStream(recbuff);

	//get request line "GET / HTTP/1.1"
	std::getline(recbuffStream, requestLine);

	//parse request line
	std::istringstream methodStream(requestLine);
	std::getline(methodStream, this->method, ' ');
	std::getline(methodStream, this->path, ' ');
	path.erase(0, 1);
	if (method == "GET") {
		response(path);
	}

}