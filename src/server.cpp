#include "server.hpp"

server::server(){}

void print_client(int client_fd, std::string str){
	send(client_fd, str.c_str(), str.size(), 0);
	std::cout << str << std::endl;
}

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
	Client newClient;
	while(true){
		_nfds = epoll_wait(_epoll_fd, _events, 10, -1);
		//TODO: ERROR MESSAGE HERE

		for(int i = 0; i < _nfds; i++){
			std::cout << "Fase 1\n";
			
			struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

			if(_events[i].data.fd == _socket_Server){
				std::cout << "Fase 2\n";
				int newsocket = accept(_socket_Server, (struct sockaddr*)&client_addr, &client_len);
				
				newClient.set_socket(newsocket);
				newClient.set_addr(client_addr);
				newClient.set_user_info(_buffer);
				// Aqui você pode adicionar os dados do usuário ao objeto client
                //std::cout << newClient.get_name() << "\n";
                //std::cout << newClient.get_pass() << "\n"; 
                //std::cout << newClient.get_nick() << "\n"; 
				//TODO: PUT ERROR MESSAGE HERE

				_eve.events = EPOLLIN;
				_eve.data.fd = newClient.get_socket();
				if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, newClient.get_socket(), &_eve) == -1){
					std::cerr << "Error adding new socket to epoll" << std::endl;
					close(newClient.get_socket());
					exit(EXIT_FAILURE);
				}
			}
			else{
				std::cout << "Fase 3\n";
				int bytes_received = recv(_events[i].data.fd, _buffer, sizeof(_buffer), 0);
					newClient.set_client_fd(_events->data.fd);
                    newClient.set_user_info(_buffer); 
				if (bytes_received <= 0){
					close (_events[i].data.fd);
					if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _events[i].data.fd, NULL) == -1) {
                        std::cerr << "Error removing socket from epoll" << std::endl;
					}
				}
				else{
					_buffer[bytes_received] = '\0';
					std::cout << "Received: " << _buffer << std::endl;
					std::string command(_buffer);
					
					handleCommands(newClient, command);
				}
			}
		}
	}
}

void server::handleCommands(Client &client_usr, const std::string &command){
	std::istringstream iss(command);
	std::string cmd;
	iss >> cmd;
	if (cmd == "auth" || cmd == "AUTH"){
		std::cout << "PASS SERVER: " << _pass << std::endl;
		std::cout << "GET PASS: " << client_usr.get_pass() << std::endl;
		std::string pass = client_usr.get_pass();
/* 		for(size_t i = 0; pass[i] != '\0'; i++){
			std::cout << "123>" << pass[i] << "<<<<<<" << std::endl;} */
		//std::cout << strcmp(pass.c_str(), _pass.c_str()) << std::endl;
		if (strcmp(pass.c_str(), _pass.c_str()) == 0){
			print_client(client_usr.get_client_fd(), "User is Authenticated\n");
			client_usr.set_auth(true);
			return ;
		}
		else{
			print_client(client_usr.get_client_fd(), "User is not Authenticated\n");
			return ;
		}

	}
	if (cmd == "join" || cmd == "JOIN"){
		if (client_usr.get_auth() == false){
			print_client(client_usr.get_client_fd(), "Need to Auth the user\n");
			return ;
		}
		std::string channelName;
		iss >> channelName;
		if (channelName.empty()){
			print_client(client_usr.get_client_fd(), "Channel name is empty\n");
			return ;
		}
		Channel *channel;
		channel = getChannel(channelName);
		if (channel == NULL){
            createChannel(channelName);
            channels[channelName]->addUser(client_usr);
            std::string creationMessage = ":" + client_usr.get_nick() + "!" + client_usr.get_name() + "@" + client_usr.get_host() + " JOIN :" + channelName + "\r\n";
			std::cout << creationMessage << std::endl;
            std::cout << "USER = " << client_usr.get_name() << " end" << "\n";
            std::cout << "USER = " << client_usr.get_pass() << " end" << "\n"; 
            std::cout << "USER = " << client_usr.get_nick() << " end" <<"\n"; 
			//print_client(client_usr.get_client_fd(), creationMessage);
            //print_client(client_usr.get_client_fd(), "Channel " + channelName + " created and user added.\n");

            // Send topic message
        	//std::string message = ":bmonteir JOIN #channelname\n";
			//send(client_usr.get_client_fd(), message.c_str(), message.length(), 0);
		    std::string channelCreate = ":" + client_usr.get_nick() + " JOIN #" + channelName + "\r\n";
            print_client(client_usr.get_client_fd(), channelCreate);
		}
		else{
            channels[channelName]->addUser(client_usr);
			std::string channelCreate = ":" + client_usr.get_nick() + " JOIN #" + channelName + "\r\n";
            print_client(client_usr.get_client_fd(), channelCreate);
		}
		/* else{
			//TODO: CHANGE THIS
			if (channel->getUser(client_fd) == client_fd){
            	channel->addUser(client_fd);
				std::string response = "User added\n";
				send(client.get_client_fd(), response.c_str(), response.size(), 0);
			}
			else{
				std::string response = "User already added\n";
				send(client.get_client_fd(), response.c_str(), response.size(), 0);
			}
		} */

        /* if (!channel) {
			createChannel(channelName);
            channel->addUser(...);
            std::string response = "Channel " + channelName + " created and user added.\n";
            send(client.get_client_fd(), response.c_str(), response.size(), 0);
        } */
	}	
}

void server::createChannel(const std::string &channelName){
	if (channels.find(channelName) == channels.end()){
		Channel *channel = new Channel(channelName);
		channels.insert(std::pair<std::string, Channel *>(channelName, channel));
	}
}

Channel *server::getChannel(const std::string name)  {
	if (channels.find(name) != channels.end())
		return channels[name];
	return NULL;
}

/* 
void server::setUsers(std::string userName){
	std::vector<std::string>::iterator it = user.  
	if (user.(userName) == user.end())
    	user.insert(user);
}

std::string const &server::getUser()const{

} */
