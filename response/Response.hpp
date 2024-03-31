/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: del-yaag <del-yaag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 15:54:22 by del-yaag          #+#    #+#             */
/*   Updated: 2024/03/31 13:01:24 by del-yaag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <vector>
#include <cstring>
#include <map>
#include <set>

#include <dirent.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/fcntl.h>
#include <errno.h>

#include "../server/Colors.hpp"
#include "../config/Location.hpp"

#define SENDED 1000000
#define CHUNKED 2621440

class Request;

class Response {

    private:
        int			receivedcontent;
        int 		statusCode;
        int 		file;
        bool		sendedHeader;
        bool 		displayError;
        bool 		autoIndexing;
        size_t		contentLength;
        size_t 		countBytesRead;
        size_t 		contentResponse;
        std::string type;
        std::string mimeType;

        // post method
        std::string body;
        std::string BHName;
        std::string BHFilename;
        std::string BHContentDispo;
        std::string BHContentType;
        std::string cgiBody;
        int         fd;
        bool        bodyFlag;

        std::string sessionId;

    public:
        Response( void );
        ~Response( void );
        int			getreceivedcontent( void ) const;
        void		setreceivedcontent( int const & receivedcontent );
        void 		setStatusCode( int const & statusCode );
        int			getStatusCode( void ) const;
        void		setSendedHeader( bool const & sendedHeader );
        bool 		getSendedHeader( void ) const;
        void 		setDisplayError( bool const & displayError );
        bool 		getDisplayError( void ) const;
        void 		setAutoIndexing( bool const & autoIndexing );
        bool 		getAutoIndexing( void ) const;
        void 		setContentLength( size_t const & contentLength );
        size_t		getContentLength( void ) const;
        void		setCountBytesRead( size_t const & countBytesRead );
        size_t		getCountBytesRead( void ) const;
        void		setContentResponse( size_t const & contentResponse );
        size_t		getContentResponse( void ) const;
        void		setFile( int const & file );
        int			getFile( void ) const;
        std::string	getType( void ) const;
        void		setType( std::string const & type );
        std::string	getMimeType( void ) const;
        void		setMimeType( std::string const & mimetype );

        ssize_t sendHeader( int const &sockfd, Request const & request);
        ssize_t sendBody( int const &sockfd, Request const & request);
        std::string getStatusMessage(int const & statusCode);
        void displayErrorPage( Conf::Server & server, int const &sockfd, Request request );
        int displayAutoIndex( Conf::Server & server, int const &sockfd, Request request );
        std::string getErrorPage(std::map<std::string, std::string> ErrorPages);
        int deleteResource(int const sockfd, Request request);
        ssize_t 	sendCgiHeader( int const sockfd, Request & request );


        // ------------ post ------------ //
        void parsePostBodyHeader( std::string const &chunck );
        int parseEncodingBody( Request const &request, Conf::Server const &server);
        int parseBoundariesBody( Request const &request, Conf::Server const &server);
        std::string getHeaderValue( std::string const &chunck, std::string const &findStr, std::string const &delim );
        int createFileAndWrite( std::string const &str, Request const &request, Conf::Server const &server, bool const flag );
        int execPostMethod( Request &request, Conf::Server const &server );
        int PutChunkedBodyToFile( Request const &request, Conf::Server const &server, bool const flag );
        void resetHeaderElements( void );
        int  openFile( Request const &request, Conf::Server const &server );
        int parseEncodingBodyCgi( Request const &request );
        int openCgiFile( std::string const &path, std::string const &body );

        int parseSessionsBody( Request &request );
        int generateSessionId( std::string &login, std::string &password, bool &loginFlag, bool &passFlag );
        std::string getSessionId( void ) const;
        void setSessionId( std::string const &id );
        
        std::string getBHName( void ) const;
        void setBHName( std::string const &name );
        std::string getBHFilename( void ) const;
        void setBHFilename( std::string const &filename );
        std::string getBHContentDispo( void ) const;
        void setBHContentDispo( std::string const &content );
        std::string getBHContentType( void ) const;
        void setBHContentType( std::string const &type );
        std::string getBody( void ) const;
        void setBody( std::string const &body );
};

std::string getMimeType(const std::string& extension);
int hexadecimalToDecimal( std::string hexVal );
std::string stringToAscii( std::string const &str );