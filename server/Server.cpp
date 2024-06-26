/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: del-yaag <del-yaag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/05 14:55:40 by del-yaag          #+#    #+#             */
/*   Updated: 2024/04/05 01:34:00 by mmisskin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "../session/Session.hpp"

Server::Server( Config const &config ) {

	this->yes = 1;
	this->notBound = 0;
	this->addrInfo = NULL;
	this->newinfo = NULL;
	this->config = config;
	this->servers = this->config.getServers();
}

Server::~Server( void ) { }

void Server::getInfoaddr( std::string const &host, std::string const &port ) {

	int status;

	std::memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	status = getaddrinfo( host.c_str(), port.c_str(), &hints, &addrInfo );
	if ( status == -1 ) {

		std::cerr << RED << "error: getaddrinfo: " << gai_strerror( status ) << RESET << std::endl;
		exit( EXIT_FAILURE );
	}
	// printvalidoption( "getaddrinfo" );
}

int Server::createsocket( int &listener ) {

	listener = socket( newinfo->ai_family, newinfo->ai_socktype, newinfo->ai_protocol );
	if ( listener == -1 ) {

		std::cerr << RED << "==> ERROR: " << strerror( errno ) << RESET << std::endl;
		return -1;
	}
	// printvalidoption( "socket" );

	if ( fcntl( listener, F_SETFL, O_NONBLOCK, FD_CLOEXEC ) == -1 ) {

		std::cerr << RED << "==> ERROR: " << strerror( errno ) << RESET << std::endl;
		return -1;
	}
	// printvalidoption( "fcntl" );

	if ( setsockopt( listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) ) == -1 ) {

		std::cerr << RED << "==> ERROR: " << strerror( errno ) << RESET << std::endl;
		return -1;
	}
	// printvalidoption( "setsockopt" );

	return 0;
}

int Server::bindlistensock( int &listener, std::vector<Conf::Server>::iterator &it ) {

	( void ) it;
	for ( newinfo = addrInfo; newinfo; newinfo = newinfo->ai_next ) {

		if ( this->createsocket( listener ) == -1 )
			continue;
		
		if ( bind( listener, newinfo->ai_addr, newinfo->ai_addrlen ) < 0 ) {

			close( listener );
			std::cerr << RED << "\t==> ERROR: " << strerror( errno ) << RESET << std::endl;
			continue;
		}
		// printvalidoption( "bind" );
		break;
	}
	freeaddrinfo( addrInfo );

	if ( !newinfo ) {

		std::string error = strerror( errno );
		printinvalidopt(    "==> ERROR: " "socket '" + intToString( listener ) +
							"' didn't get bound with '" + it->getListen().getHost() +
							":" + it->getListen().getPort() + "' because: " + error ); // not bound address error
		this->notBound++;
		return 0;
	}

	if ( listen( listener, BACKLOG ) == -1 ) {
		
		std::cerr << RED << "\t==> ERROR: " << strerror( errno ) << RESET << std::endl; // listen error
		exit( EXIT_FAILURE );
	}
	// else
	// 	printvalidoption( "listen" ); // valid listen
	serverfds[listener] = *it;
	return 1;
}

int Server::alreadyboundsock( std::vector<Conf::Server>::iterator const &server ) {

	if ( this->donehp.size() > 0 ) {
		
		std::pair<std::string, std::string> search = std::make_pair( server->getListen().getHost(), server->getListen().getPort() );
		std::set<std::pair<std::string, std::string> >::iterator it = donehp.find( search );
		if ( it != donehp.end() ) {

			printinvalidopt( "==> Seems like this address is already bound " + it->first + ":" + it->second );
			std::cout << std::endl;
			return 1;
		}
	}
	return 0;
}

void Server::createServer( void ) {

	int listener;  

	std::vector<Conf::Server>::iterator it = servers.begin();
	for ( ; it != servers.end(); ++it ) {

		if ( this->alreadyboundsock( it ) ) // check already bound sockets and ignore them
			continue;
		
		this->getInfoaddr( it->getListen().getHost(), it->getListen().getPort() ); // create socket
		if ( this->bindlistensock( listener, it ) ) { // bind socket
			
			donehp.insert( std::make_pair( it->getListen().getHost(), it->getListen().getPort() ) ); // add bound host, port to the container
			std::cout << DYELLO << "\tserver: " << it->getListen().getHost() << ":" << it->getListen().getPort() << RESET << std::endl << std::endl; // valid server bound
		}

	}

	if ( this->notBound == servers.size() ) { // cannot bind any socket

		printinvalidopt( "===> ERORR CANNOT BIND ANY SOCKET. it seems like all socket in use try with another config!!!" );
		exit( EXIT_FAILURE );
	}
	
	this->addpollservers();

	while ( 1 )
		this->mainpoll();
}

void Server::addpollservers( void ) {

	struct pollfd pfd;
	std::map<int, Conf::Server>::iterator it = serverfds.begin();
	for ( ; it != serverfds.end(); ++it ) {

		pfd.fd = it->first; 
		pfd.events = POLLIN;
		pfds.push_back( pfd );
	}
}

void Server::addpollclients( int const &fd ) {

	std::vector<struct pollfd>::iterator it = this->pfds.begin();
	for ( ; it != pfds.end(); it++ )
		if ( it->fd == fd )
			return ;
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = POLLIN;
	pfds.push_back( pfd );
}

void Server::addclients( int const &sockfd, Conf::Server const &server ) {

	Client client;
	std::map<int, Client>::iterator it;

	client.setsockfd( sockfd );
	client.settimeout( std::time(NULL) );
	client.setDefaultServer( server );
	client.setserver( server );
	client.setConfig( config );

	if ( clients.size() == 0 ) {

		std::cout << DMAGENTA << "\t-> add first client" << RESET << std::endl;
		clients[sockfd] = client;
	} else {

		it = clients.find( sockfd );
		if ( it == clients.end() ) {

			std::cout << MAGENTA << "\t-> add more clients" << RESET << std::endl;
			clients[sockfd] = client;
		}
	}
}

int Server::acceptconnections( int const &sockfd, Conf::Server server ) {

	int newfd;

	this->addrlen = sizeof( this->remoteaddr );
	newfd = accept( sockfd, ( struct sockaddr * )&remoteaddr, &addrlen );
	if ( newfd == -1 ) {

		if ( errno == ECONNABORTED || errno == EAGAIN ) {

			std::cerr << RED << "==> ERROR: " << strerror( errno ) << RESET << std::endl;
			return -1;
		}
		std::cerr << RED << "accept fail" << RESET << std::endl;
		return (-1);
	} else {

		if ( fcntl( sockfd, F_SETFL, O_NONBLOCK, FD_CLOEXEC ) == -1 ) {

			std::cerr << RED << "==> ERROR: " << strerror( errno ) << RESET << std::endl;
			std::cerr << RED << "fcntl fail" << RESET << std::endl;
			return (-1);
		}
		this->addpollclients( newfd );
		this->addclients( newfd, server );
		this->printConeectedaddr( server, newfd );
	}
	return 0;
}

void Server::pollwithtimeout( void ) {

	int status;

	status = poll( pfds.data(), pfds.size(), TIMEOUT );
	if ( status == -1 ) {

		std::cerr << RED << "==> ERROR: " << strerror( errno ) << RESET << std::endl;
		exit( EXIT_FAILURE );
	} else if ( !status )
		this->checkclienttimeout();
}

void Server::mainpoll( void ) {

    std::map<int, Conf::Server>::iterator it;
    std::map<int, Client>::iterator itClient;
    
    this->pollwithtimeout();
    // std::cout << "poll: " << pfds.size() << std::endl;
    // std::cout << "client: " << clients.size() << std::endl;
    for ( size_t i = 0; i < pfds.size(); i++ ) {
        it = serverfds.find( pfds[i].fd );
        itClient = clients.find( pfds[i].fd );
        if ( pfds[i].revents == POLLIN ) {
            if ( it != serverfds.end() ) {
                if ( this->acceptconnections( pfds[i].fd, it->second ) == -1 )
                    continue;
            } else {

                // POLLIN revent int the clients side
                if ( itClient->second.recieveRequest() == 0 ) {
                    pfds[i].events = POLLOUT;
                }
            }
        } else if ( pfds[i].revents == POLLOUT ) {

            if ( it != serverfds.end() ) {

                // POLLOUT revent in the server side
            } else {
                // POLLOUT revents in the clients side
                int var;
                itClient->second.settimeout( std::time(NULL) );
                var = itClient->second.sendresponse();
                if ( !var ) {
                  
                    this->removeclient( pfds[i].fd );
                    this->removepollclient( i );
                }
                else if (var == 2) {
                  
                    pfds[i].events = POLLIN;
                    Client client(itClient->second);
                    this->removeclient( pfds[i].fd );
                    clients[pfds[i].fd] = client;
                }
            }
        } else if ( pfds[i].revents == POLLHUP ) {
          
            close(itClient->second.getResponse().getFile());
            
           if (itClient->second.getRequest().isCgi())
		   {
				remove(itClient->second.getRequest().getCgi().getCgiOutFile().c_str());
				if (itClient->second.getRequest().getMethod() == "POST") {
					remove(itClient->second.getRequest().getCgi().getCgiInFile().c_str());
					close(itClient->second.getRequest().getCgi().getCgiStdErr());
				}
		   }
                
            this->removeclient( pfds[i].fd );
            this->removepollclient( i );
        } else {
            
            if ( itClient != clients.end() && !itClient->second.getRequest().getRecString().empty() && !itClient->second.getEndRecHeader() ) { // bad request

                itClient->second.getResponse().setStatusCode( 400 );
                pfds[i].events = POLLOUT;
            }
            this->checkclienttimeout();
        }
        Session::deleteSessionIdTimeOut();
    }
}

void Server::checkclienttimeout( void ) {

    if ( !clients.empty() ) {

        std::map<int, Client>::iterator it = clients.begin();
        std::time_t now = std::time(NULL);
        std::time_t diff;
        std::time_t start;
        for ( ; it != clients.end(); ++it ) {

            start = it->second.gettimeout();
            diff = now - start;
            if ( diff >= 10 ) {

                this->searchandremovepollclient( it->second.getsockfd() );
                close(it->second.getResponse().getFile());
                clients.erase( it );
                printinvalidopt( "-> client has been deleted " );
                if ( clients.empty() )
                    break;
                it = clients.begin();
            }
        }
    }
}

void Server::searchandremovepollclient( int const &sockfd ) {

	std::vector<struct pollfd>::iterator it = pfds.begin();
	for ( ; it != pfds.end(); ++it )
		if ( it->fd == sockfd )
			break;
	if ( it != pfds.end() ) {

		close( it->fd );
		pfds.erase( it );
		printinvalidopt( "-> connection closed with the client TIMEOUT" );
	}
}

void Server::removeclient( int const &sockfd ) {

	std::map<int, Client>::iterator it = clients.find( sockfd );
	if ( it != clients.end() ) {

		clients.erase( it->first );
		std::cout << MAGENTA << "\t->client has been deleted" << RESET << std::endl;
	}
}

void Server::removepollclient( int const &index ) {

	std::vector<struct pollfd>::iterator it = pfds.begin() + index;
	close( pfds[index].fd );
	pfds.erase( it );
	std::cout << MAGENTA << "\t->poll client has been deleted" << RESET << std::endl;
}

void printvalidoption( std::string const &str ) {

	std::cout << GREEN << "\t--> " << str << RESET << std::endl;
}

void printinvalidopt( std::string const &str ) {

	std::cerr << RED << "\t" << str << RESET << std::endl;
}

void Server::printConeectedaddr ( Conf::Server const &server, int const &sockfd ) {

	std::cout
		<< DBLUE 
		<< "\t--> connection accepted: "
		<< server.getListen().getHost()
		<< ":"
		<< server.getListen().getPort()
		<< " on "
		<< sockfd
		<< RESET
		<< std::endl;
}
