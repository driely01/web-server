/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiScript.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: del-yaag <del-yaag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 03:10:20 by del-yaag          #+#    #+#             */
/*   Updated: 2024/03/31 11:31:59 by del-yaag         ###   ########.fr       */
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

class ParseData {

    private:
        std::string body;
        std::string name;
        std::string filename;
        std::string startBoundary;
        std::string endBoundary;
        std::string login;
        std::string password;
        bool islogin;
        bool ispassword;
    
    public:
        ParseData( void );
        ~ParseData( void );
        std::string getBody( void ) const;
        std::string getLogin( void ) const;
        void setLogin( std::string const &login );
        std::string getPassword( void ) const;
        void setPassword( std::string const &password );
        void setBody( std::string const &body );
        std::string getStartBoundary( void ) const;
        void getStartEndBoundaries( void );
        void setStartBoundary( std::string const &boundary );
        std::string getEndBoundary( void ) const;
        void setEndBoundary( std::string const &boundary );
        int parseBody( void );
        int parseLogin( void );
        void parsePostBodyHeader( std::string const &chunck );
        std::string getHeaderValue( std::string const &chunck, std::string const &findStr, std::string const &delim );
};
