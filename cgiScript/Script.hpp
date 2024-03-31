/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Script.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amoukhle <amoukhle@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/31 02:30:46 by amoukhle          #+#    #+#             */
/*   Updated: 2024/03/31 03:45:57 by amoukhle         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <unistd.h>
#include <string>
#include <algorithm>
#include <fstream>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <sys/fcntl.h>

std::string LoginPage( void );
std::string welcomePage( std::string login );
std::string errorPage();