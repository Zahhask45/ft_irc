#include "server.hpp"

server::server(){}

server::server(int port, std::string pass) : _port(port), _pass(pass), _nfds(1){
	//! DONT KNOW WHERE TO PUT THIS
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1) {
		std::cerr << "Error creating epoll file descriptor" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	_socket_Server = socket(AF_INET, SOCK_STREAM, 0);

	int opt = 1;
	setsockopt(_socket_Server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

server::~server() {}

int const &server::get_socket() const{
	return _socket_Server;
}

void server::binding(){
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = INADDR_ANY;
	_addr.sin_port = htons(_port);
	bind(_socket_Server, (const struct sockaddr *)&_addr, sizeof(_addr));
	listen(_socket_Server, 10);

	//! DONT KNOW WHERE TO PUT THIS
	_eve.events = EPOLLIN;
	_eve.data.fd = _socket_Server;

	if(epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _socket_Server, &_eve) == -1){
		std::cerr << "Error adding socket to epoll" << std::endl;
		close(_socket_Server);
		exit(EXIT_FAILURE);
	}
}

void server::loop(){
	while(true){
		_nfds = epoll_wait(_epoll_fd, _events, 10, -1);
		//TODO: ERROR MESSAGE HERE

		for(int i = 0; i < _nfds; i++){
			struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

			if(_events[i].data.fd == _socket_Server){
				int newsocket = accept(_socket_Server, (struct sockaddr*)&client_addr, &client_len);
				
				client newClient();
				//clients.set_socket(newsocket);
				newClient.
				//TODO: PUT ERROR MESSAGE HERE

				_eve.events = EPOLLIN;
				_eve.data.fd = clients.get_socket();
				if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, clients.get_socket(), &_eve) == -1){
					std::cerr << "Error adding new socket to epoll" << std::endl;
					close(clients.get_socket());
					exit(EXIT_FAILURE);
				}
			}
			else{
				int bytes_received = recv(_events[i].data.fd, _buffer, sizeof(_buffer), 0);
				if (bytes_received <= 0){
					close (_events[i].data.fd);
					if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _events[i].data.fd, NULL) == -1) {
                        std::cerr << "Error removing socket from epoll" << std::endl;
					}
				}
				else{
					for (size_t i = 0; _buffer[i] != '\0'; i++){
						std::cout << _buffer[i] << std::endl;
					}
					
					_buffer[bytes_received] = '\0';
					std::cout << "Received: " << _buffer << std::endl;
					std::string command(_buffer);
					handleCommands(_events->data.fd, command);
				}
			}
		}
	}
}

void server::handleCommands(int client_fd, const std::string &command){
	std::istringstream iss(command);
	std::string cmd;
	iss >> cmd;
	
	if (cmd == "join" || cmd == "JOIN"){
		std::string channelName;
		iss >> channelName;
		if (channelName.empty()){
			std::string response = "Error: Channel name is required.\n";
			send(client_fd, response.c_str(), response.size(), 0);
			return ;
		}
		Channel *channel = new Channel();
		channel = getChannel(channelName);
		if (channel == NULL){
			createChannel(channelName, *channel);
			//! ERROR HERE NOT client_fd, NEED TO CREATE A USER FOR THE client_fd
            channel->addUser(client_fd);
			std::string response = "Channel created and user added\n";
			send(client_fd, response.c_str(), response.size(), 0);
		}
		/* else{
			//TODO: CHANGE THIS
			if (channel->getUser(client_fd) == client_fd){
            	channel->addUser(client_fd);
				std::string response = "User added\n";
				send(client_fd, response.c_str(), response.size(), 0);
			}
			else{
				std::string response = "User already added\n";
				send(client_fd, response.c_str(), response.size(), 0);
			}
		} */

        /* if (!channel) {
			createChannel(channelName);
            channel->addUser(client_fd);
            std::string response = "Channel " + channelName + " created and user added.\n";
            send(client_fd, response.c_str(), response.size(), 0);
        } */
		
	}

	
}

void server::createChannel(const std::string &channelName, Channel &channel){
	if (channels.find(channelName) == channels.end()){
		channels.insert(std::pair<std::string, Channel *>(channelName, &channel));
	}
}

Channel *server::getChannel(const std::string name)  {
	if (channels.find(name) != channels.end())
		return channels[name];
	return NULL;
}


void server::setUsers(std::string userName){
	std::vector<std::string>::iterator it = user.  
	if (user.(userName) == user.end())
    	user.insert(user);
}

std::string const &server::getUser()const{

}




/* std::map<int, client> clients;

void server::loop(){
    while(true){
        _nfds = epoll_wait(_epoll_fd, _events, 10, -1);
        //TODO: ERROR MESSAGE HERE

        for(int i = 0; i < _nfds; i++){
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            if(_events[i].data.fd == _socket_Server){
                int newsocket = accept(_socket_Server, (struct sockaddr*)&client_addr, &client_len);
                //TODO: Extrair informações do usuário do buffer aqui
                client newClient(newsocket, client_addr, informações do usuário );
                clients[newsocket] = newClient;

                _eve.events = EPOLLIN;
                _eve.data.fd = newsocket;
                if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, newsocket, &_eve) == -1){
                    std::cerr << "Error adding new socket to epoll" << std::endl;
                    close(newsocket);
                    exit(EXIT_FAILURE);
                }
            }
            else{
                int bytes_received = recv(_events[i].data.fd, _buffer, sizeof(_buffer), 0);
                if (bytes_received <= 0){
                    close (_events[i].data.fd);
                    if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _events[i].data.fd, NULL) == -1) {
                        std::cerr << "Error removing socket from epoll" << std::endl;
                    }
                    clients.erase(_events[i].data.fd);
                }
                else{
                    for (size_t i = 0; _buffer[i] != '\0'; i++){
                        std::cout << _buffer[i] << std::endl;
                    }
                    
                    _buffer[bytes_received] = '\0';
                    std::cout << "Received: " << _buffer << std::endl;
                    std::string command(_buffer);
                    handleCommands(_events->data.fd, command);
                }
            }
        }
    }
} */