#include "Server.hpp"

void Server::response( std::string path, std::vector<clients_t>::iterator& itclient ) {
	char buffer[SEND];

	std::string type = path.substr(path.rfind('.') + 1);
	std::string mimeType = getMimeType(type);
	if (itclient->contentResponse == 0) {
		itclient->message = itclient->statusLine + "\r\nContent-Type: " + mimeType + "\r\nContent-Length: ";
		itclient->message.append( std::to_string( itclient->contentLength ) ).append( "\r\nAccept-Ranges: bytes" );
		if (!itclient->rangeStart.empty())
			itclient->message.append( "\r\nContent-Range: bytes " ).append( itclient->rangeStart ).append("-").append( itclient->rangeEnd ).append("/*");
		itclient->message.append( "\r\n\r\n" );
		itclient->contentLength += itclient->message.length();
		itclient->contentResponse += itclient->message.length();
		// std::cout << "message : " << itclient->message << std::endl;
	}
	else
	{
		size_t bytesRead = read(itclient->file, buffer, SEND);
		if (bytesRead < SEND)
			buffer[bytesRead] = '\0';
		itclient->contentResponse += bytesRead;
		itclient->message = std::string(buffer, bytesRead);
	}
	//////---------display request info---------///////
	// std::cout << "path: " << path << std::endl;
	// std::cout << "type: " << type << std::endl;
	// std::cout << "mimetype: " << mimeType << std::endl;
}
void Server::sendresponse( int const &i ) {

	ssize_t sended;
	std::vector<clients_t>::iterator itclients;

	if ( pfds[i].fd != listener ) {
		itclients = this->findActiveClient( i );
		if ( itclients != clients.end() ) {

			// method
			if (itclients->method == "GET") {
				response(itclients->path, itclients);
			}

			// send header response
			if ( itclients->message.length() < SEND ) {
				sended = send( pfds[i].fd, ( itclients->message.c_str() ), itclients->message.length(), 0 );
			}
			else { //send content
				sended = send( pfds[i].fd, ( itclients->message.c_str() ), SEND, 0 );
			} 
			if ( sended == -1 ) {
				std::cerr << "failed to send" << std::endl;
			} else {
				if ( itclients->contentResponse == itclients->contentLength ) {
					// std::cout << "content Response: " << itclients->contentResponse << std::endl;
					std::cout << "---> sent: " << pfds[i].fd << std::endl;

					// remove client
					this->removeclient( itclients );

					// remove pollf
					this->removepollsock( i );
				}
			}
		}
	}
}
