/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   postMethod.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: del-yaag <del-yaag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/28 01:06:39 by del-yaag          #+#    #+#             */
/*   Updated: 2024/03/31 13:01:48 by del-yaag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "../server/Server.hpp"
#include "../request/Request.hpp"
#include "../session/Session.hpp"

std::string Response::getBHName( void ) const {
    
    return this->BHName;
}

void Response::setBHName( std::string const &name ) {
    
    this->BHName = name;
}

std::string Response::getBHFilename( void ) const {
    
    return this->BHFilename;
}

void Response::setBHFilename( std::string const &filename ) {
    
    this->BHFilename = filename;
}

std::string Response::getBHContentDispo( void ) const {
    
    return this->BHContentDispo;
}

void Response::setBHContentDispo( std::string const &content ) {
    
    this->BHContentDispo = content;
}

std::string Response::getBHContentType( void ) const {
    
    return this->BHContentType;
}

void Response::setBHContentType( std::string const &type ) {
    
    this->BHContentType = type;
}

std::string Response::getBody( void ) const {

    return this->body;
}

void Response::setBody( std::string const &body ) {

    this->body = body;
}

std::string Response::getSessionId( void ) const {
    
    return this->sessionId;
}

void Response::setSessionId( std::string const &id ) {
    
    this->sessionId = id;
}

std::string  Response::getHeaderValue( std::string const &chunck, std::string const &findStr, std::string const &delim ) {

    size_t find;
    size_t next;
    std::string str;
    
    find = chunck.find( findStr );
    if ( find != std::string::npos ) {
        
        find += findStr.size();
        next = chunck.find( delim, find );
        if ( next != std::string::npos )
            str = chunck.substr( find, next - find );
    }
    
    return str;
}

void  Response::parsePostBodyHeader( std::string const &chunck ) {
    
    this->BHContentDispo = this->getHeaderValue( chunck, "Content-Disposition: ", ";" ); // get content desposition
    this->BHName = this->getHeaderValue( chunck, "name=\"", "\"" ); // get name
    this->BHFilename = this->getHeaderValue( chunck, "filename=\"", "\"" ); // get filename
    this->BHContentType = this->getHeaderValue( chunck, "Content-Type: ", "\r\n" ); // get content type
}

int  Response::createFileAndWrite( std::string const &str, Request const &request, Conf::Server const &server, bool const flag ) {

    std::string path;
    ssize_t bytes;

    if ( flag == true ) {
        
        if ( !this->getBHFilename().empty() ) {

            bytes = write( this->fd, str.c_str(), str.size() );
            if ( bytes == -1 ) {

                std::cerr << RED << "\tERROR: open: cannot open " << path << RESET << std::endl;
                close( this->fd );
                this->setStatusCode( 500 );
                return 0;
            }
        }
        
    } else {
        
        if ( !this->getBHFilename().empty() ) {
            
            if ( request.getCheckLocation() ) { // get location path

                if ( request.getLocation().getUploadPath().empty() || request.getLocation().getUploadPath().getPath() == "/" ) {

                    std::cerr << RED << "\tServer Upload directory not set!" << RESET << std::endl;
                    this->setStatusCode( 500 );
                    return 0;
                }
                if ( request.getLocation().getUploadPath().getPath() != "/" ) {

                    if ( request.getLocation().getUploadPath().getPath().front() == '/' )
                        path = request.getLocation().getUploadPath().getPath() + "/" + this->getBHFilename(); // absolute path
                    else
                        path = request.getLocation().getRoot().getPath() + "/" + request.getLocation().getUploadPath().getPath() + "/" + this->getBHFilename(); // relative path
                }
            }
            else { // get server path

                if ( server.getUploadPath().empty() || server.getUploadPath().getPath() == "/" ) {

                    std::cerr << RED << "\tServer Upload directory not set!" << RESET << std::endl;
                    this->setStatusCode( 500 );
                    return 0;
                }
                if ( server.getUploadPath().getPath() != "/" ) {

                    if ( server.getUploadPath().getPath().front() == '/' )
                        path = server.getUploadPath().getPath() + "/" + this->getBHFilename(); // absolute path
                    else
                        path = server.getRoot().getPath() + "/" + server.getUploadPath().getPath() + "/" + this->getBHFilename(); // relative path
                }
            } 

            this->fd = open( path.c_str(), O_WRONLY | O_CREAT, 0644 );
            if ( fd == -1 ) {
                std::cerr << RED << "\tERROR: open: cannot open " << path << RESET << std::endl;
                this->setStatusCode( 500 );
                return 0;
            }
            bytes = write( this->fd, str.c_str(), str.size() );
            if ( bytes == -1 ) {
                
                std::cerr << RED << "\tERROR: write: cannot write in " << path << RESET << std::endl;
                close( this->fd );
                this->setStatusCode( 500 );
                return 0;
            }
        }
    }
    return 1;
}

int  Response::openFile( Request const &request, Conf::Server const &server ) {

    std::string path;
        
    if ( request.getCheckLocation() ) { // get location path

        if ( request.getLocation().getUploadPath().empty() || request.getLocation().getUploadPath().getPath() == "/" ) {

            std::cerr << RED << "\tServer Upload directory not set!" << RESET << std::endl;
            this->setStatusCode( 500 );
            return 0;
        }
        if ( request.getLocation().getUploadPath().getPath() != "/" ) {

            if ( request.getLocation().getUploadPath().getPath().front() == '/' )
                path = request.getLocation().getUploadPath().getPath() + "/" + this->getBHFilename(); // absolute path
            else
                path = request.getLocation().getRoot().getPath() + "/" + request.getLocation().getUploadPath().getPath() + "/" + this->getBHFilename(); // relative path
        }
    }
    else { // get server path

        if ( server.getUploadPath().empty() || server.getUploadPath().getPath() == "/" ) {

            std::cerr << RED << "\tServer Upload directory not set!" << RESET << std::endl;
            this->setStatusCode( 500 );
            return 0;
        }
        if ( server.getUploadPath().getPath() != "/" ) {

            if ( server.getUploadPath().getPath().front() == '/' )
                path = server.getUploadPath().getPath() + "/" + this->getBHFilename(); // absolute path
            else
                path = server.getRoot().getPath() + "/" + server.getUploadPath().getPath() + "/" + this->getBHFilename(); // relative path
        }
    }

    this->fd = open( path.c_str(), O_WRONLY | O_CREAT, 0644 );
    if ( fd == -1 ) {
        std::cerr << RED << "\tERROR: open: cannot open " << path << RESET << std::endl;
        this->setStatusCode( 500 );
        return 0;
    }
    return 1;
}

int Response::openCgiFile( std::string const &path, std::string const &body ) {

    int fd = open( path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644 );
    if ( fd == -1 ) {

        std::cerr << RED << "\tERROR: open: cannot open " << path << RESET << std::endl;
        this->setStatusCode( 500 );
        return 0;
    }
    int bytes = write( fd, body.c_str(), body.size() );
    if ( bytes == -1 ) {

        close( fd );
        std::cerr << RED << "\tERROR: write: cannot write in " << path << RESET << std::endl;
        this->setStatusCode( 500 );
        return 0;
    }
    close( fd );;
    return 1;
}

void Response::resetHeaderElements( void ) {

    this->setBHContentDispo( "" );
    this->setBHContentType( "" );
    this->setBHFilename( "" );
    this->setBHName( "" );
}

int Response::PutChunkedBodyToFile( Request const &request, Conf::Server const &server, bool const flag ) {

    std::string chunked;
    std::string chunkedBody;
    size_t find;
    size_t next;

    chunkedBody = this->body.substr( 0, CHUNKED );
    find = chunkedBody.find( request.getStartBoundary() );
    if ( find != std::string::npos ) {

        next = chunkedBody.find( "\r\n", find );
        chunked = chunkedBody.substr( find, next - find );
        if ( chunked == request.getEndBoundary() ) { // got the end boundaries

            if ( !this->bodyFlag && !this->BHFilename.size() ) {

                this->setStatusCode( 501 );
                return 1;
            }

            chunked = chunkedBody.substr( 0, find - 2 );
            if ( !this->createFileAndWrite( chunked, request, server, flag ) )
                return 2; // error
            this->body.erase( 0, find ); // erase body
            this->resetHeaderElements(); // reset all vars
            if ( flag )
                this->bodyFlag = false;
            close( this->fd );
            
            return 0; // end

        } else if ( chunked == request.getStartBoundary() ) { // got the start boundaries
            
            chunked = chunkedBody.substr( 0, find - 2 );
            if ( !this->createFileAndWrite( chunked, request, server, flag ) )
                return 2;
            this->body.erase( 0, find ); // erase body
            this->resetHeaderElements(); // reset all vars
            if ( flag )
                this->bodyFlag = false;
            close( this->fd );
        }
    } else {

        if ( !this->createFileAndWrite( chunkedBody, request, server, flag ) )
            return 2;
        this->body.erase( 0, CHUNKED ); // erase body
        if ( !flag )
            this->bodyFlag = true;
    }
    return 1;
}

int Response::generateSessionId( std::string &login, std::string &password, bool &loginFlag, bool &passFlag ) {
    
    std::string chunked;
    size_t find;

    if ( ( this->BHName == "login" && !loginFlag) || ( this->BHName == "password" && !passFlag ) ) {
                    
        find = this->body.find( "\r\n" );
        if ( find != std::string::npos ) {

            chunked = this->body.substr( 0, find ); // get the login or password value
            this->body.erase( 0, find + 2 );
        }
        if ( this->BHName == "login" ) {
            
            loginFlag = true;
            login = stringToAscii( chunked );
        }
        else if ( this->BHName == "password" ) {
            
            passFlag = true;
            password = stringToAscii( chunked );
        }
        if ( loginFlag && passFlag )
            this->sessionId = login + password;
    } else {
        return 0; // error
    }
    return 1;
}

int Response::parseSessionsBody( Request &request ) {
    
    std::string buffer;
    std::string chunked;
    std::string login;
    std::string password;
    size_t find;
    bool loginFlag = false;
    bool passFlag = false;

    this->cgiBody = this->body;
    while ( !this->body.empty() ) {
        
        find = this->body.find( "\r\n" );
        if ( find != std::string::npos ) {
            
            buffer = this->body.substr( 0, find );
            
            if ( buffer == request.getEndBoundary() ) { // end of the body

                if ( !loginFlag || !passFlag ) {
                    
                    request.setCookie( "" );
                    this->sessionId = "";
                    return 0;
                }
                break;
            } else if ( buffer == request.getStartBoundary() ) { // get body header

                this->body.erase( 0, find + std::strlen( "\r\n" ) );
                find = this->body.find( "\r\n\r\n" );
                if ( find != std::string::npos ) { // get header.

                    chunked = this->body.substr( 0, find );
                    this->parsePostBodyHeader( chunked ); // parse the header body
                }
                this->body.erase( 0, find + std::strlen( "\r\n\r\n" ) );
            }
            if ( !this->BHName.empty() && this->BHFilename.empty() ) { // get the login or password
                    
                if ( !this->generateSessionId( login, password, loginFlag, passFlag ) ) { // generate the session id

                    this->sessionId = "";
                    request.setCookie( "" );
                    return 0; // error
                }
            }
            else if ( !this->getBHFilename().empty() ) { // ignore files
                
                this->sessionId = "";
                request.setCookie( "" );
                return 0; // error
            }
        }
    }
    if ( !Session::findSessionId( request.getCookie() ) ) { // not find cookie's id in the sessions
    
        Session::addSession( this->sessionId );
        request.setCookie( "" );
    } else { // found cookie's id in the sessions
    
        this->sessionId = "";
    }
    return 1;
}

int  Response::parseBoundariesBody( Request const &request, Conf::Server const &server ) {

    std::string buffer;
    std::string chunked;
    std::string chunkedBody;
    size_t find;
    int status;

    if ( this->bodyFlag == false ) {

        find = this->body.find( "\r\n" );
        if ( find != std::string::npos ) {
            
            buffer = this->body.substr( 0, find );
            
            if ( buffer == request.getEndBoundary() ) {

                if ( !this->bodyFlag && !this->BHFilename.size() ) {

                    this->setStatusCode( 501 );
                    return 1;
                }
                return 0;
            }
            else if ( buffer == request.getStartBoundary() ) {

                this->body.erase( 0, find + std::strlen( "\r\n" ) );
                find = this->body.find( "\r\n\r\n" );
                if ( find != std::string::npos ) { // get header.

                    chunked = this->body.substr( 0, find );
                    this->parsePostBodyHeader( chunked );
                    if ( !this->getBHFilename().empty() )
                        std::cout << GREEN << "\tsending... " << this->getBHFilename() << RESET << std::endl;
                }
                this->body.erase( 0, find + std::strlen( "\r\n\r\n" ) );

                status = this->PutChunkedBodyToFile( request, server, false );
                if ( !status )
                    return 0;
                else if ( status == 2 )
                    return 1;
                else if ( status == 3 ) // for database cgi
                    return 3;
            }
        }
    } else {
        
        status = this->PutChunkedBodyToFile( request, server, true );
        if ( !status )
            return 0;
        else if ( status == 2 )
            return 1;
    }
    return 1;
}

int Response::parseEncodingBodyCgi( Request const &request ) {

    std::string buffer;
    std::string chunked;
    size_t lengthToRead;
    size_t find;
    find = this->body.find( "\r\n" );
    if ( find != std::string::npos ) {
        
        buffer = this->body.substr( 0, find );
        lengthToRead = hexadecimalToDecimal( buffer );
        this->body.erase( 0, find + 2 );
        chunked = this->body.substr( 0, lengthToRead );
        this->cgiBody += chunked;
        this->body.erase( 0, lengthToRead + 2 );
        if ( chunked.find( request.getEndBoundary() ) != std::string::npos ) {
            
            this->body = this->cgiBody;
            // SET FLAG 
            return 0;
        }
    }
    return 1;
}

int Response::parseEncodingBody( Request const &request, Conf::Server const &server ) {

    std::string buffer;
    std::string chunked;
    size_t lengthToRead;
    size_t find;
    ( void )request;

    find = this->body.find( "\r\n" );
    if ( find != std::string::npos ) {

        buffer = this->body.substr( 0, find );
        lengthToRead = hexadecimalToDecimal( buffer );
        this->body.erase( 0, find + 2 );
        chunked = this->body.substr( 0, lengthToRead );
        if ( chunked.find( request.getStartBoundary() ) != std::string::npos &&
            (   chunked.find( "name=" ) != std::string::npos ||
                chunked.find( "filename=" ) != std::string::npos ||
                chunked.find( "Content-Type: " ) != std::string::npos ) ) { // body header

            this->resetHeaderElements(); // reset vars
            this->parsePostBodyHeader( chunked );
            if ( !this->getBHFilename().empty() )
                std::cout << GREEN << "\tsending... " << this->getBHFilename() << RESET << std::endl;
            close( this->fd );
            this->bodyFlag = false;

            this->body.erase( 0, lengthToRead + 2 );

        } else if ( chunked.find( request.getEndBoundary() ) != std::string::npos ) { // end of body

            if ( !this->bodyFlag && !this->BHFilename.size() ) { // if body is empty

                this->setStatusCode( 501 );
                return 1;
            } else if ( !this->bodyFlag && this->BHFilename.size() ) {

                if ( !this->openFile( request, server ) )
                    return 1;
            }
            this->resetHeaderElements();
            close( this->fd );
            return 0;

        } else { // chunked body

            if ( !this->bodyFlag ) {

                if ( !this->createFileAndWrite( chunked, request, server, false ) ) // first time create file
                    return 1;
                this->bodyFlag = true;
            } else {

                if ( !this->createFileAndWrite( chunked, request, server, true ) ) // just write file already exits
                    return 1;
            }
            this->body.erase( 0, lengthToRead + 2 );
        }
    }
    return 1;
}

int Response::execPostMethod( Request &request, Conf::Server const &server ) {

    int status;
    if ( request.isCgi() ) {

        if ( request.getBodyType() == ENCODING ) {
            if ( !this->parseEncodingBodyCgi( request ) ) { // parse body by removing enconding
              
                this->parseSessionsBody( request );
                if ( !this->openCgiFile( request.getCgi().getCgiInFile(), this->cgiBody ) ) // open file and put the body inside it
                    return 1; // error
                // cgi work here
				request.getCgi().setReady(true);
                return 0; // send response to the client
            }
        } else if ( request.getBodyType() == BOUNDARIES || request.getBodyType() == LENGTH ) {
            
            this->parseSessionsBody( request );
            if ( !this->openCgiFile( request.getCgi().getCgiInFile(), this->cgiBody ) ) // open file and put the body inside it
                return 1; //error
            // cgi work here
			request.getCgi().setReady(true);
            return 0; // send response to the client
        }
        
    } else {
        
        if ( request.getBodyType() == ENCODING )
            status = this->parseEncodingBody( request, server );
        else if ( request.getBodyType() == BOUNDARIES )
            status = this->parseBoundariesBody( request, server );
        if ( request.getConnection() == "close" && !status )
			return 0;
        else if ( !status )
			return 2;
    }
    return 1;
}

// ---------------------------------- //
int hexadecimalToDecimal( std::string hexVal ) {

    int len = hexVal.size();
    int base = 1;
    int dec_val = 0;

    for (int i = len - 1; i >= 0; i--) {

        if (hexVal[i] >= '0' && hexVal[i] <= '9') {

            dec_val += (int(hexVal[i]) - 48) * base;
            base = base * 16;
            
        } else if (hexVal[i] >= 'a' && hexVal[i] <= 'f') {

            dec_val += (int(hexVal[i]) - 87) * base;
            base = base * 16;
        } else if (hexVal[i] >= 'A' && hexVal[i] <= 'F') {

            dec_val += (int(hexVal[i]) - 55) * base;
            base = base * 16;
        } 
    }
    return dec_val;
}

std::string stringToAscii( std::string const &str ) {

    std::string ascii;

    for ( size_t i = 0; i < str.size(); i++ ) {

        std::stringstream stream;
        stream << static_cast<int>( str[i] );
        ascii += stream.str();
    }

    return ascii;
}
