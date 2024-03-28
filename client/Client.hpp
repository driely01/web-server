/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: del-yaag <del-yaag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 15:54:46 by del-yaag          #+#    #+#             */
/*   Updated: 2024/03/24 02:31:46 by mmisskin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../response/Response.hpp"
#include "../request/Request.hpp"
#include "../config/Server.hpp"
#include "../config/Config.hpp"

#include <ctime>

class Server;

class Client {

    private:
        int sockfd;
        std::time_t timeout;
        Conf::Server server;
        Config config;
        Request request;
        Response response;
        bool endRecHeader;


    public:
        Client( void );
        ~Client( void );
        Client(Client const & src);
        Client	&operator=(Client const & obj);
        int getsockfd( void ) const;
        void setsockfd( int const &sockfd );
        std::time_t gettimeout( void ) const;
        void settimeout( std::time_t const &timeout );
        Conf::Server getserver( void ) const;
        void setserver( Conf::Server const &server );
        Config getConfig( void );
        void setConfig( Config config );
        bool getEndRecHeader( void );
        void setEndRecHeader( bool endRecHeader );
        int sendPostResponse( std::string const &message );
        int recieveRequest();
        int sendresponse();
};

size_t stringToInt(const std::string& str);
std::string intToString(size_t num);
size_t get_size_fd(std::string fileD);
bool isDirectory(const char* path);
bool isRegularFile(const char* path);
