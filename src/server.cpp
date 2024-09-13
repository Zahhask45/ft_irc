#include "server.hpp"

server::server(){}

void print_client(int client_fd, std::string str){
	send(client_fd, str.c_str(), str.size(), 0);
}

server::server(int port, std::string pass) : _port(port), _pass(pass), _nfds(1), _cur_online(0){
	//! DONT KNOW WHERE TO PUT THIS
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1) {
		std::cerr << "Error creating epoll file descriptor" << std::endl;
		exit(EXIT_FAILURE);
	}
}

server::~server() {}

int const &server::get_socket() const{
	return _socket_Server;
}

void server::binding(){
	struct addrinfo hints, *serverinfo, *tmp;
	int status;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = getprotobyname("TCP")->p_proto;

	std::stringstream ss;
	int opt = 1;
	ss << _port;
	std::string Port = ss.str();
	status = getaddrinfo("0.0.0.0", Port.c_str(), &hints, &serverinfo);
	if (status != 0){
		std::cout << "ERROR ON GETTING ADDRESS INFO" << std::endl;
		exit (-1);
	}
	for (tmp = serverinfo; tmp != NULL; tmp = tmp->ai_next){
		this->_socket_Server = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
		if (this->_socket_Server < 0)
			continue;
		setsockopt(this->_socket_Server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		if (bind(_socket_Server, tmp->ai_addr, tmp->ai_addrlen) < 0){
			close(this->_socket_Server);
			continue;
		}
		break;
	}

	freeaddrinfo(serverinfo);

	if (tmp == NULL) {
		std::cerr << "Failed to bind to any address" << std::endl;
		exit(EXIT_FAILURE);
	}
	

	if (listen(_socket_Server, 10) == -1) {
    	std::cerr << "Error in listen()" << std::endl;
    	exit(EXIT_FAILURE);
	}

	_events[0].data.fd = _socket_Server;
	_events[0].events = EPOLLIN;
	
	if(epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _socket_Server, _events) == -1){
		std::cerr << "Error adding socket to epoll" << std::endl;
		close(_socket_Server);
		exit(EXIT_FAILURE);
	}
	_cur_online++;
}

void server::loop(){
	while(true){

		std::cout << "FUCK" <<std::endl;
		_nfds = epoll_wait(_epoll_fd, _events, 10, -1);
		if (_nfds == -1){
			std::cout << "epoll_wait() error" << std::endl;
			exit (-1);
		}
		for(int i = 0; i < _cur_online; i++){
			if (_events[i].events & EPOLLIN){

				if(_events[i].data.fd == _socket_Server){
					std::cout << "Fase 2\n";
					
					struct sockaddr_storage client_addr;
					socklen_t client_len = sizeof(client_addr);
					int newsocket = accept(_socket_Server, (struct sockaddr*)&client_addr, &client_len);
					if (newsocket == -1){
						std::cout << "Error on accept" << std::endl;
					}
					//! fcntl(newsocket, F_SETFL, O_NONBLOCK);
					
					_events[_cur_online].data.fd = newsocket;
					_events[_cur_online].events = EPOLLIN;
					this->clients.insert(std::pair<int, Client *>(newsocket, new Client(newsocket)));
					clients[newsocket]->set_addr(client_addr);


					epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, newsocket, &_events[_cur_online]);


					this->_cur_online++;
				}
				else{
					std::cout << "Fase 3\n";
					int bytes_received = recv(_events[i].data.fd, _buffer, sizeof(_buffer), 0);
					if (bytes_received <= 0){
						std::cout << "Fase 3.1\n";
						close (_events[i].data.fd);
						if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _events[i].data.fd, NULL) == -1) {
							std::cerr << "Error removing socket from epoll" << std::endl;
						}
					}
					else{
						std::cout << "Fase 3.2\n";
						_buffer[bytes_received] = '\0';
						std::string command(_buffer);
						std::cout << "Received: " << command << std::endl;
						/* if (!command.empty() && command[command.size() - 1] == '\r') {
        					command.erase(command.end() - 1);
						} */
						handleCommands(_events[i].data.fd, command);

					}
					memset(_buffer, 0, 1024);
				}
			}
		}
	}
}

//! VERIFY AMOUNT OF ARGUMENTS PASS TO THE COMMANDS
void server::handleCommands(int fd, const std::string &command){
	std::istringstream iss(command);
	std::string cmd;
	iss >> cmd;
	std::cout << command << std::endl;
	if (cmd == "auth" || cmd == "AUTH"){
		std::cout << "PASS SERVER: " << _pass << std::endl;
		std::cout << "GET PASS: " << clients[fd]->get_pass() << std::endl;
		std::string pass = clients[fd]->get_pass();
		for(size_t i = 0; pass[i] != '\0'; i++){
			std::cout << "123>" << pass[i] << "<<<<<<" << std::endl;}
		std::cout << strcmp(pass.c_str(), _pass.c_str()) << std::endl;
		if (strcmp(pass.c_str(), _pass.c_str()) == 0){
			print_client(fd, "User is Authenticated\n");
			clients[fd]->set_auth(true);
			return ;
		}
		else{
			print_client(fd, "User is not Authenticated\n");
			return ;
		}

	}

	if (cmd == "pass" || cmd == "PASS"){
		std::string pass;
		iss >> pass;
		if (_pass != pass){
			print_client(clients[fd]->get_client_fd(), "WRONG PASSWORD\n");
			return ;
		}

		clients[fd]->set_pass(pass);
		print_client(clients[fd]->get_client_fd(), "PASSWORD CONFIRMED\n");
	}





	if (cmd == "nick" || cmd == "NICK"){
		std::string nick;
		iss >> nick;
		
		clients[fd]->set_nick(nick);
		print_client(clients[fd]->get_client_fd(), "NICK SETTED\n");
		if (clients[fd]->get_user() != "\0"){
			clients[fd]->set_mask(clients[fd]->get_nick() + "!" + clients[fd]->get_user() + "@" + clients[fd]->get_host());
		}
	}

	if (cmd == "user" || cmd == "USER"){
		std::string user;
		iss >> user;
		
		clients[fd]->set_user(user);
		print_client(clients[fd]->get_client_fd(), "USER SETTED\n");
		if (clients[fd]->get_nick() != "\0"){
			clients[fd]->set_mask(clients[fd]->get_nick() + "!" + clients[fd]->get_user() + "@" + clients[fd]->get_host());
		}
	}







	if (cmd == "join" || cmd == "JOIN"){
		if (clients[fd]->get_auth() == false){
			print_client(clients[fd]->get_client_fd(), "Need to Auth the user\n");
			return ;
		}
		std::string channelName;
		iss >> channelName;
		if (channelName.empty()){
			print_client(clients[fd]->get_client_fd(), "Channel name is empty\n");
			return ;
		}
		Channel *channel;
		channel = getChannel(channelName);
		if (channel == NULL){
			createChannel(channelName);
			channels[channelName]->addUser(getClient(fd));
			std::string creationMessage = ":" + clients[fd]->get_mask() + " JOIN :#" + channelName + "\r\n";
			std::cout << creationMessage << std::endl;

			print_client(clients[fd]->get_client_fd(), creationMessage);
			clients[clients[fd]->get_client_fd()]->addChannel(channelName, *channel);

			print_client(clients[fd]->get_client_fd(), "Channel " + channelName + " created and user added.\n");

			// Send topic message
			std::string topicMessage = ":server 332 " + clients[fd]->get_nick() + " " + channelName + " :Welcome to " + channelName + "\r\n";
			print_client(clients[fd]->get_client_fd(), topicMessage);
			return ;
		}
		else{
			std::string creationMessage = ":" + clients[fd]->get_mask() + " JOIN :#" + channelName + "\r\n";
		}

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


Client &server::getClient(int fd){
	std::map<int, Client *>::iterator it = clients.find(fd);
	return *it->second;
}

/* 
void server::setUsers(std::string userName){
	std::vector<std::string>::iterator it = user.  
	if (user.(userName) == user.end())
    	user.insert(user);
}

std::string const &server::getUser()const{

} */
