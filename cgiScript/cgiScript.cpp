/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiScript.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: del-yaag <del-yaag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 03:10:41 by del-yaag          #+#    #+#             */
/*   Updated: 2024/03/31 12:32:19 by del-yaag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cgiScript.hpp"

ParseData::ParseData( void ) {

    this->islogin = false;
    this->ispassword = false;
}

ParseData::~ParseData( void ) { }

std::string ParseData::getLogin( void ) const {
    
    return this->login;
}

void ParseData::setLogin( std::string const &login ) {
    
    this->login = login;
}

std::string ParseData::getPassword( void ) const {
    
    return this->password;
}

void ParseData::setPassword( std::string const &password ) {
    
    this->password = password;
}

std::string ParseData::getBody( void ) const {

    return this->body;
}

void ParseData::setBody( std::string const &body ) {

    this->body = body;
}

std::string ParseData::getStartBoundary( void ) const {
    
    return this->startBoundary;
}
void ParseData::setStartBoundary( std::string const &boundary ) {
    
    this->startBoundary = boundary;
}

std::string ParseData::getEndBoundary( void ) const {
    
    return this->endBoundary;
}

void ParseData::setEndBoundary( std::string const &boundary ) {
    
    this->endBoundary = boundary;
}

void ParseData::getStartEndBoundaries( void ) {

    size_t find;

    find = this->body.find( "\r\n" );
    if ( find != std::string::npos ) {

        this->startBoundary = this->body.substr( 0, find );
        this->endBoundary = this->getStartBoundary().append( "--" );
    }
}

std::string  ParseData::getHeaderValue( std::string const &chunck, std::string const &findStr, std::string const &delim ) {

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

void  ParseData::parsePostBodyHeader( std::string const &chunck ) {
    
    this->name = this->getHeaderValue( chunck, "name=\"", "\"" ); // get name
    // std::cout << "name: " << this->name << std::endl;
    this->filename = this->getHeaderValue( chunck, "filename=\"", "\"" ); // get filename
    // std::cout << "filename: " << this->filename << std::endl;
}

int ParseData::parseBody( void ) {
    
    std::string buffer;
    std::string chunked;
    size_t find;

    while ( !this->body.empty() ) {
        
        find = this->body.find( "\r\n" );
        if ( find != std::string::npos ) {
            
            buffer = this->body.substr( 0, find );
            
            if ( buffer == this->endBoundary ) { // end of the body
                
                if ( !this->islogin || !this->ispassword )
                    return 0; // error missing password or login
                return 1;
            } 
            else if ( buffer == this->startBoundary ) { // get body header

                this->body.erase( 0, find + std::strlen( "\r\n" ) );
                find = this->body.find( "\r\n\r\n" );
                if ( find != std::string::npos ) { // get header.

                    chunked = this->body.substr( 0, find );
                    this->parsePostBodyHeader( chunked ); // parse the header body
                }
                this->body.erase( 0, find + std::strlen( "\r\n\r\n" ) );
            }
            if ( !this->name.empty() && this->filename.empty() ) { // get the login or password

                if ( !this->parseLogin() )
                    return 0; // error found duplicate login, password or invalid fieled
            }
            else if ( !this->filename.empty() ) // error found a file
                return 0;
        }
    }
    return 1;
}

int ParseData::parseLogin( void ) {
    
    std::string chunked;
    size_t find;

    if ( ( this->name == "login" && !this->islogin ) || ( this->name == "password" && !this->ispassword ) ) {
        
        find = this->body.find( "\r\n" );
        if ( find != std::string::npos ) {
            
            chunked = this->body.substr( 0, find ); // get the login or password value
            this->body.erase( 0, find + 2 );
            if ( this->name == "login" ) {
                
                this->islogin = true;
                this->login =  chunked ;
            }
            else if ( this->name == "password" ) {
                
                this->ispassword = true;
                this->password =  chunked;
            }
        }
    } else
        return 0; // error
    return 1;
}
