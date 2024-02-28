#include "Server.hpp"

void	Server::parseRequest( char *recievebuff, int const &i ) {
	std::string recbuff(recievebuff);
	std::string requestLine;
	std::istringstream recbuffStream(recbuff);
	std::string method;
	std::string path;

	//get request line "GET / HTTP/1.1"
	std::getline(recbuffStream, requestLine);

	//parse request line
	std::istringstream methodStream(requestLine);
	std::getline(methodStream, method, ' ');
	std::getline(methodStream, path, ' ');
	path.erase(0, 1);
	if (method == "GET") {
		response(path, i);
	}
}