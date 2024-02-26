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
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags = AI_PASSIVE;

    this->status = getaddrinfo( "10.11.6.6", PORT, &hints, &ai );
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

        std::cout << "didn't get bound" << std::endl;
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
    std::cout << "-------> sdd server sock to poll" << std::endl;
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
    std::cout << "poll removed" << std::endl;
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
    }
    std::cout << "-----> accept" << std::endl;
    return 0;
}

// clients funtions 
void Server::addClient( int const &sockfd ) {

    clients_t client;
    std::map<int, clients_t>::iterator it;

    client.sockfd = sockfd;
    client.content = 0;

    if ( clients.size() == 0 ) {

        std::cout << "add the first client" << std::endl;
        clients[sockfd] = client;
    } else {

        it = clients.find( sockfd );
        if ( it == clients.end() ) {

            std::cout << "add more client" << std::endl;
            clients[sockfd] = client;
        }
    }
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
        } else if ( pfds[i].revents & POLLOUT ) {

            // send the response to the client
            this->sendresponse( i );
        }
    }
}

// ---------------------------- test ---------------------------- //
std::string Server::response( void ) {

    file.open( "min.mp4", std::ios::in );
    if ( !file ) {

        std::cout << "failed to open file" << std::endl;
        exit( EXIT_FAILURE );
    }
    while ( getline( file, buffer ) ) {

        tmp.append( buffer ).append( "\n" );
    }

    message = "HTTP/1.1 200 OK\r\nContent-Type: video/mp4\r\nContent-Length: ";
    message.append( std::to_string( tmp.size() ) ).append( "\r\n\r\n" ).append( tmp );
    std::cout << "message length: " << message.length() << std::endl;

    return message;
}

void Server::recieverequest( int const &i ) {

    std::map<int, clients_t>::iterator itclients;
    itclients = clients.find( pfds[i].fd );
    if ( itclients != clients.end() ) {

        char recievebuff[SIZE];
        int sender = pfds[i].fd;
        int recieved = recv( pfds[i].fd, recievebuff, SIZE, 0 );
        recievebuff[recieved] = 0;
        if ( recieved <= 0 ) {

            if ( recieved == 0 )
                std::cout << "*-*> pollserver: socket " << sender << " hung up" << std::endl;
            else
                perror( "recv" );

            clients.erase( itclients );
            this->removepollsock( i );
        }  else {

            std::cout << recievebuff << std::endl;
            pfds[i].events = POLLOUT;
            std::cout << pfds[i].fd << " " << itclients->first << " " << "----> pullout" << std::endl;
        }
    }
}



void Server::sendresponse( int const &i ) {

    int deff;
    ssize_t sended;
    std::vector<struct pollfd>::iterator itpfds;
    std::map<int, clients_t>::iterator itclients;

    if ( pfds[i].fd != listener ) {

        itclients = clients.find( pfds[i].fd );
        if ( itclients != clients.end() ) {

            // here is the response place
            if ( ( itclients->second.content + SEND ) >=  message.length() ) {

                if ( itclients->second.content == 0 )
                    deff =  message.length();
                else 
                    deff = message.length() - itclients->second.content;
                sended = send( pfds[i].fd, ( message.c_str() + itclients->second.content ), deff, 0 );
            } else
                sended = send( pfds[i].fd, ( message.c_str() + itclients->second.content ), SEND, 0 );
            if ( sended == -1 ) {

                perror( "send" );
                this->clients.erase( itclients );
            } else {

                itclients->second.content += sended;
                // std::cout << itclients->sockfd << " " << itclients->content << " => " << message.length() << std::endl;
                if ( itclients->second.content >= message.length() ) {

                    std::cout << "---> sent" << std::endl;

                    // remove client
                    this->clients.erase( itclients );
                    // remove pollf
                    this->removepollsock( i );

                }
            }
        }
    }
}