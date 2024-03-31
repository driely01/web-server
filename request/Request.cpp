/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: del-yaag <del-yaag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 15:54:29 by del-yaag          #+#    #+#             */
/*   Updated: 2024/03/31 12:58:51 by del-yaag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "../response/Response.hpp"
#include "../client/Client.hpp"
#include "../session/Session.hpp"

Request::Request( void ) {

    this->sendedcontent = 0;
    this->checkLocation = false;
    this->returnCode = -1;
}

Request::~Request( void ) { }

int Request::getsendedcontent( void ) const {

    return this->sendedcontent;
}

void Request::setsendedcontent( int const &sendedcontent ) {

    this->sendedcontent = sendedcontent;
}

void Request::setRecString( std::string const & recString ) {

    this->recString += recString;
}
std::string Request::getRecString( void ) const {

    return (this->recString);
}

void Request::setMethod( std::string const & method ) {

    this->method = method;
}
std::string Request::getMethod( void ) const {

    return (this->method);
}

void Request::setPath( std::string const & path ) {

    this->path = path;
}
std::string Request::getPath( void ) const {

    return (this->path);
}

std::string Request::getHeader( void ) const {

    return (this->header);
}
void Request::setHeader( std::string const & header ) {

    this->header = header;
}

std::string Request::getBody( void ) const {

    return (this->body);
}

void Request::setBody( char const *body, int const &size ) {

    if (this->body.empty())
        this->body.append( body, size );
    else
        this->body.append( body, size );
}

void Request::setBodyForCgi( std::string const &body ) {

    this->body = body;
}

std::string Request::getConnection( void ) const {

    return (this->connection);
}
void Request::setConnection( std::string const & connection ) {

    this->connection = connection;
}

size_t Request::getRequestBodySize( void ) const {

    std::string subString = "\r\n\r\n";

    size_t found = recString.find(subString);
    if (found != std::string::npos) {
        return (recString.length() - (found + subString.length()));
    }
    else
        return (0);
}

std::string Request::getRangeStart( void ) const {

    return (this->rangeStart);
}
void Request::setRangeStart( std::string const & rangeStart ) {
    this->rangeStart = rangeStart;
}
std::string Request::getRangeEnd( void ) const {

    return (this->rangeEnd);
}
void Request::setRangeEnd( std::string const & rangeEnd ) {

    this->rangeEnd = rangeEnd;
}

size_t Request::getRangeStartNum( void ) const {

    return (this->rangeStartNum);
}
void Request::setRangeStartNum( size_t const & rangeStartNum ) {

    this->rangeStartNum = rangeStartNum;
}
size_t Request::getRangeEndNum( void ) const {

    return (this->rangeEndNum);
}
void Request::setRangeEndNum( size_t const & rangeEndNum ) {

    this->rangeEndNum = rangeEndNum;
}

void Request::setHttpVersion ( std::string const & httpVersion ) {

    this->httpVersion = httpVersion;
}
std::string Request::getHttpVersion( void ) const {

    return (this->httpVersion);
}

std::map<std::string, std::string> const & Request::getLinesRequest( void ) const {

    return (this->linesRequest);
}

Location const & Request::getLocation( void ) const {

    return (this->location);
}
void Request::setLocation( Location const & location ) {

    this->location = location;
}
bool Request::getCheckLocation( void ) const {

    return (this->checkLocation);
}
void Request::setCheckLocation( bool checkLocation ) {

    this->checkLocation = checkLocation;
}

std::string Request::getStringLocation( void ) const {

    return (this->stringLocation);
}
void Request::setStringLocation( std::string const & stringLocation ) {

    this->stringLocation = stringLocation;
}

std::string Request::getUrl( void ) const {

    return (this->url);
}
void Request::setUrl( std::string url ) {

    this->url = url;
}

int         Request::getReturnCode( void ) const {
    
    return (this->returnCode);
}
void        Request::setReturnCode( int returnCode ) {
    
    this->returnCode = returnCode;
}
std::string Request::getReturnUrl( void ) const {
    
    return (this->returnUrl);
}
void        Request::setReturnUrl( std::string const & returnUrl ) {
    
    this->returnUrl = returnUrl;
}

std::string Request::getCookie( void ) const {
    
    return (this->cookie);
}
void    Request::setCookie( std::string const cookie ) {
    
    this->cookie = cookie;
}

int Request::setRequestHeader( void ) {

    std::string subString = "\r\n\r\n";

    size_t found = recString.find(subString);
    if (found != std::string::npos) {
        this->header = recString.substr(0, found + subString.length());
        std::cout << BLUE << this->header << RESET << std::endl;
        return (1);
    }
    else
        return (0);
}

void Request::setRequestBody( void ) {

    std::string subString = "\r\n\r\n";

    size_t found = recString.find(subString);
    this->body = recString.substr(found + subString.length());

}

void		Request::setCgiFiles(std::string const & suffix) { cgi.setFiles(suffix); }
bool		Request::isCgi(void) const { return (cgi.isSet()); }
Cgi &		Request::getCgi(void) { return (cgi); }

int Request::parseRequestLine( Config conf, Conf::Server & server, Response & response ) {

    std::string requestLine;
	std::istringstream headerStream(this->header);

    std::string host = getValue("Host:");
    server = conf.getServer(server, host);

    //get request line "GET / HTTP/1.1"
	std::getline(headerStream, requestLine);

    //parse request line
	std::istringstream methodStream(requestLine);

    // get method | path | httpVersion
	std::getline(methodStream, this->method, ' ');
	std::getline(methodStream, this->path, ' ');
  
    this->url = this->path;
    std::getline(methodStream, this->httpVersion, '\r');
    // check httpVersion is valid
    if (this->httpVersion != "HTTP/1.1") {
        response.setStatusCode( 505 );
        return (0);
    }
    return (1);
}

int Request::checkMethod( Response & response ) {

    if (this->checkLocation && !location.getLimitExcept().empty()) {
        std::set<std::string> setMethods = location.getLimitExcept().getMethods();
        std::set<std::string>::iterator itMethod = setMethods.find(this->method);
        if (itMethod == setMethods.end()) {
            response.setStatusCode( 405 );
            return (0);
        }
    }
    else if (this->method != "GET" && this->method != "POST" && this->method != "DELETE"){
        response.setStatusCode( 405 );
        return (0);
    }
    return (1);
}

int Request::setMapRequestLines( Response & response ) {

    std::string line;
    std::string key;
    std::string value;
    std::istringstream headerStreamtmp(this->header);
    std::getline(headerStreamtmp, line);
    while (std::getline(headerStreamtmp, line) && line != "\r") {
        std::istringstream lineStream(line);
        std::getline(lineStream, key, ' ');
        std::getline(lineStream, value, '\r');
        if (value.empty() || key[key.length() - 1] != ':') {
            response.setStatusCode( 400 );
            return (0);
        }
        this->linesRequest[key] = value;
    }
    return (1);
}

int Request::parseRequestHeader( Config conf, Conf::Server & server, Response & response ) {

    if ( !this->parseRequestLine(conf, server, response) )
        return (0);

    // get location
    std::map<std::string, Location>::iterator itLocation = server.getLocation(this->path);
    if (itLocation != server.getLocations().end()) {
        this->location = itLocation->second;
        this->stringLocation = itLocation->first;
        this->checkLocation = true;
    }

    // get return
    if (this->checkLocation) {
        this->returnCode = this->location.getReturn().getCode();
        this->returnUrl = this->location.getReturn().getUrl();
    }
    else {
        this->returnCode = server.getReturn().getCode();
        this->returnUrl = server.getReturn().getUrl();
    }

    // check method is valid !!!!!!
    if ( !this->checkMethod( response ) )
        return (0);
    
    // get cookie
    this->cookie = getValue("Cookie:");
    size_t find = this->cookie.find( "id=" );
    if ( find != std::string::npos )
        this->cookie = this->cookie.substr( find + 3, this->cookie.find( ";" ) - ( find + 3 ) );

    if ( !Session::findSessionId( this->cookie ) ) // search for the session id
        this->setCookie( "" );

    // get lines of request
    if (!this->setMapRequestLines( response ) )
        return (0);

    // get connection
    std::map<std::string, std::string>::iterator it = this->linesRequest.find("Connection:");
    if (it != this->linesRequest.end())
        this->connection = it->second;
    else
        this->connection = "close";

    if ( this->method == "POST" )
        this->parsePostHeader();

	// detect cgi requests
    if (this->checkLocation && !itLocation->second.getCgiPass().empty())
	{
		size_t 		extension = this->path.find('.');
		std::string	cgiExtension;

		if (extension != std::string::npos)
			cgiExtension = this->path.substr(extension, this->path.find_first_of("?/", extension) - extension);
		std::cout << GREEN << "-------> " << cgiExtension << RESET << std::endl;
		if (itLocation->second.getCgiPass().found(cgiExtension))
		{
			if (!handleCgiRequest(itLocation->second.getRoot().getPath(), itLocation->first, itLocation->second.getCgiPass().getCgi(cgiExtension), response))
				return (0);
			std::cout << CYAN << "found cgi: [" << cgiExtension << "] -> " << itLocation->second.getCgiPass().getCgi(cgiExtension) << RESET <<std::endl;
			this->cgi.enable();
			return (1);
		}
	}

    if (this->method == "GET") {
        // check path is valid !!!!!
        this->path.erase(0, 1);
        std::string absolutPAth;
        if (this->checkLocation)
            absolutPAth = this->location.getRoot().getPath() + this->url;
        else
            absolutPAth = server.getRoot().getPath() + this->url;
        if (this->path.empty() || isDirectory(absolutPAth.c_str())) {

            if ( !this->checkDirectory( server, response ) )
                return (0);
        }
        else {
            if ( !this->checkFile( server, response ) )
                return (0);
        }

        // get content length
        response.setContentLength( get_size_fd(this->path) );

        //get Range
        this->getRange();
    }
    else if (this->method == "DELETE") {
        this->path.erase(0, 1);
        if ( !this->checkFile( server, response ) )
                return (0);
	}

    return (1);
}

std::string Request::getValue( std::string const & key ) const {

    std::string line;
    std::istringstream recbuffStream(recString);

    while (std::getline(recbuffStream, line))
    {
        std::string part;
        std::istringstream lineStream(line);
        std::getline(lineStream, part, ' ');
        if (part == key) {
            std::getline(lineStream, part, '\r');
            return (part);
        }
    }
    return ("");
}

std::string Request::getIndex( std::vector<std::string> const & indexes, std::string const & root ) const {

    std::string fullIndex;
    for (int i = 0; i < (int)indexes.size(); i++) {
        fullIndex = root + "/" + indexes[i];
        if (!access(fullIndex.c_str(), F_OK)) {
            return (fullIndex);
        }
    }
    return ("");
}
