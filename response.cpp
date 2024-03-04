#include "Server.hpp"

void	Server::sendHeader( std::vector<clients_t>::iterator& itclient, int const &i ) {

	// *-------header response-------* //
		// ---------type && mime type-------- //
		std::string type = itclient->path.substr(itclient->path.rfind('.') + 1);
		std::string mimeType = getMimeType(type);

		// ----------status line----------- //
		if (access(itclient->path.c_str(), F_OK) == -1) {
			itclient->statusLine = "HTTP/1.1 404 Not Found";
			itclient->notFound = true;
		}
		else {
			itclient->statusLine = "HTTP/1.1 200 OK";
			if (!itclient->rangeStart.empty())
				itclient->statusLine = "HTTP/1.1 206 Partial Content";
		}

		itclient->headerResponse = itclient->statusLine + "\r\nContent-Type: " + mimeType + "\r\nContent-Length: ";
		if (!itclient->rangeStart.empty()) {
			itclient->headerResponse += intToString( itclient->rangeEndNum - itclient->rangeStartNum + 1 ) + "\r\nAccept-Ranges: bytes";
			itclient->headerResponse += "\r\nContent-Range: bytes " + itclient->rangeStart + "-" + itclient->rangeEnd + "/" + intToString( itclient->contentLength );
			itclient->contentLength = itclient->rangeEndNum - itclient->rangeStartNum + 1;
		}
		else
		{
			itclient->headerResponse += intToString( itclient->contentLength );
			if (mimeType.substr(0, mimeType.find('/')) == "video" || mimeType.substr(0, mimeType.find('/')) == "audio")
				itclient->headerResponse += "\r\nAccept-Ranges: bytes";
		}
		// itclient->headerResponse += "\r\nConnection: keep-alive";
		itclient->headerResponse += "\r\n\r\n";

		// std::cout << "header response: " << itclient->headerResponse << std::endl;
		// ---------send header response---------- //
		ssize_t sended;
		sended = send( pfds[i].fd, ( itclient->headerResponse.c_str() ), itclient->headerResponse.length(), 0 );
		if ( sended == -1 || itclient->notFound ) {

			perror( "send" );
			std::cout << "aaach had zmar : " << pfds[i].fd << std::endl;
			this->removeclient( itclient );
			this->removepollsock( i );
		}
		else
			itclient->sentHeader = true;
	// *-------header response-------* //

}

void	Server::sendBody( std::vector<clients_t>::iterator& itclient, int const &i ) {
	// *---------body response-------* //

		// -----------open file---------- //
		if (itclient->file == -2) {
			itclient->file = open( itclient->path.c_str(), O_RDONLY, 0777 );
			if ( itclient->file == -1 ) {
				std::cerr << "failed to open file" << std::endl;
				exit( EXIT_FAILURE );
			}
		}

		char buffer[SEND];
		char bufferS[1000000];
		ssize_t sended;

		// --------seek the file-------- //
		if ( !itclient->rangeStart.empty() && itclient->countBytesRead < itclient->rangeStartNum ) {
			size_t bufferSize = 1000000;
			if (itclient->rangeStartNum - itclient->countBytesRead < 1000000)
				bufferSize = itclient->rangeStartNum - itclient->countBytesRead;
			size_t bytesRead = read(itclient->file, bufferS, bufferSize);
			itclient->countBytesRead += bytesRead;
			// std::cout << "HERE : " << itclient->countBytesRead << std::endl;
		}
		else { // --------read file to send----------//
			size_t bytesRead = read(itclient->file, buffer, SEND);
			if (bytesRead < SEND)
				buffer[bytesRead] = '\0';
			itclient->contentResponse += bytesRead;
			itclient->message = std::string(buffer, bytesRead);
			sended = send( pfds[i].fd, ( itclient->message.c_str() ), itclient->message.length(), 0 );
			if ( sended == -1 ) {

				perror( "send" );
				std::cout << "aaach had zmar" << std::endl;
			}
			// std::cout << "fd: " << pfds[i].fd << " : " << itclient->contentResponse << " - " << itclient->contentLength << std::endl;
			if ( itclient->contentResponse == itclient->contentLength ) {

				std::cout << "---> sent: " << pfds[i].fd << std::endl;

				// remove client
				this->removeclient( itclient );

				// remove pollf
				this->removepollsock( i );
			}
		}
	// *---------body response-------* //
}

void Server::sendresponse( int const &i ) {

	std::vector<clients_t>::iterator itclients;

	if ( pfds[i].fd != listener ) {
		itclients = this->findActiveClient( i );
		if ( itclients != clients.end() ) {

			// method
			if (itclients->method == "GET") {
				if (itclients->sentHeader)
					sendBody( itclients, i );
				else
					sendHeader( itclients, i );
				
			}
		}
	}
}
