#include "Server.hpp"

// default constructor
Server::Server( void ) {

    this->yes = 1;
    this->ai = NULL;
    this->p = NULL;
}

// destructor
Server::~Server( void ) { }

// this fuction must be removed at the last of the project just for debugging 
void *Server::getinaddr( struct sockaddr *sa ) {

    if ( sa->sa_family == AF_INET )
        return &( ( struct sockaddr_in * )sa )->sin_addr;
    return &( ( struct sockaddr_in6 * )sa )->sin6_addr;
}

// get address info and store them in struct addrinfo to use them later
void Server::getInfoAddr( void ) {

    std::memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    this->status = getaddrinfo( NULL, PORT, &hints, &ai );
    if ( this->status != 0 ) {

        std::cout << "error: getaddrinfo: " << gai_strerror( status ) << std::endl;
        exit( EXIT_FAILURE );
    }
    std::cout << "-> getaddrinfo" << std::endl;
}

// create the socket and make it nonblocking and reusabel
int Server::createsocket( void ) {

    listener = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
        if ( this->listener < 0 ) {

            perror( "socket" );
            return -1;
        }
        std::cout << "--> socket" << std::endl;

        if ( fcntl( listener, F_SETFD, O_NONBLOCK ) == -1 ) {

            perror( "fcntl" );
            return -1;
        }
        std::cout << "---> fcntl" << std::endl;

        if ( setsockopt( listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) ) == -1 ) {

            perror( "setsockopt" );
            return -1;
        }
        std::cout << "----> setsockopt" << std::endl;

    return 0;
}

// bind the socket and listen on it
void Server::socketlistener( void ) {

    for ( p = ai; p; p = p->ai_next ) {

        if ( this->createsocket() == -1 )
            continue;
        
        if ( bind( listener, p->ai_addr, p->ai_addrlen ) < 0 ) {

            close( listener );
            perror( "bind" );
            continue;
        }
        std::cout << "-----> bind" << std::endl;

        break;
    }
    freeaddrinfo( ai );
    if ( !p ) {

        std::cerr << "didn't get bound" << std::endl;
        exit( EXIT_FAILURE );
    }

    if ( listen( listener, BACKLOG ) == -1 ) {

        perror( "listen" );
        exit( EXIT_FAILURE );
    }
    std::cout << "------> listen" << std::endl;

    // add server sockts to poll container
    this->addServersToPoll();
}

void Server::addServersToPoll( void ) {

    // this fuction need some work this just for one socket.
    struct pollfd pfd;

    pfd.fd = this->listener;
    pfd.events = POLLIN;
    pfds.push_back( pfd );
    std::cout << "-------> add server sock to poll" << std::endl;
}

// add client poll
void Server::addPollClient( int const &sockfd ) {
    struct pollfd pfd;

    pfd.fd = sockfd;
    pfd.events = POLLIN;
    pfds.push_back( pfd );
}

// remove poll socket
void Server::removepollsock( int const &i ) {

	close( pfds[i].fd );
	this->pfds.erase( pfds.begin() + i );
}

// accept connections
int Server::acceptConnections( void ) {

    int newsockfd;

    this->addrlen = sizeof( this->remoteaddr );
    newsockfd = accept( listener, ( struct sockaddr *)&remoteaddr, &addrlen );
    if ( newsockfd == -1 ) {

        if ( errno == ECONNABORTED || errno == EAGAIN ) {

            perror( "accept" );
            return -1;
        }
        exit( EXIT_FAILURE );
    } else {

        this->addPollClient( newsockfd );
        this->addClient( newsockfd );
        this->printConeectedaddr( newsockfd ); // for debugging
    	std::cout << "-----> accept: " << newsockfd << std::endl;
    }
    return 0;
}

// clients funtions 
void Server::addClient( int const &sockfd ) {

    clients_t client;
    std::vector<clients_t>::iterator it;

    client.sockfd = sockfd;
    client.content = 0;
	client.contentLength = 0;
	client.contentResponse = 0;

    if ( clients.size() == 0 ) {

        std::cout << "add the first client" << std::endl;
        clients.push_back( client );
    } else {


        it = clients.begin();
        for ( ; it != clients.end(); ++it )
            if ( it->sockfd == sockfd )
                break;
        if ( it == clients.end() ) {

            std::cout << "add more client" << std::endl;
            clients.push_back( client );
        }
    }

}

std::vector<clients_t>::iterator Server::findActiveClient( int const &i ) {

    std::vector<clients_t>::iterator itclients;
    if ( clients.size() > 0 ) {

        itclients = clients.begin();
        for ( ; itclients != clients.end(); ++itclients ) {

            if ( itclients->sockfd == pfds[i].fd )
                break;
        }
    }
    return itclients;
}

void Server::removeclient( std::vector<clients_t>::iterator const &it ) {

    clients.erase( it );
}

void Server::printConeectedaddr( int const &sockfd ) {

    std::cout
        << "pollserver: new connection from: "
        << inet_ntop(
                        remoteaddr.ss_family, 
                        this->getinaddr( ( struct sockaddr * )&remoteaddr ),
                        remoteIp, INET6_ADDRSTRLEN
                    )
        << " on "
        << sockfd
        << std::endl;
}

void Server::pollMainWork( void ) {

	if ( poll( pfds.data(), pfds.size(), -1 ) == -1 ) {
		perror( "poll" );
		exit( EXIT_FAILURE );
	}

	for ( int i = 0; i < ( int )pfds.size(); i++ ) {
		if ( pfds[i].revents & POLLIN ) {
			if ( pfds[i].fd == listener ) {
				if ( this->acceptConnections() == -1 )
					continue;
			}
			else {
				// read request and parse... and change the event socket to pollout
				this->recieverequest( i );
			}
		} else if ( pfds[i].revents == POLLOUT ) {
			// send the response to the client
			this->sendresponse( i );
		}
	}
}
