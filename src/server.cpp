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
					fcntl(newsocket, F_SETFL, O_NONBLOCK);
					
					_events[i].data.fd = newsocket;
					_events[i].events = EPOLLIN;
					this->clients.insert(std::pair<int, Client *>(newsocket, new Client(newsocket)));
					clients[newsocket]->set_addr(client_addr);


					epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, newsocket, &_events[i]);


					this->_cur_online++;
				}
				else{
					std::cout << "Fase 3\n";
					int bytes_received = recv(_events[i].data.fd, _buffer, sizeof(_buffer), 0);
					if (bytes_received <= 0){
						std::cout << "Fase 3.1\n";
						//close (_events[i].data.fd);
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
			print_client(clients[fd]->get_client_fd(), ":BANANA 001 " + nick + " " + "WELCOME\r\n");
		}
	}

	if (cmd == "user" || cmd == "USER"){
		std::string user;
		iss >> user;
		
		clients[fd]->set_user(user);
		print_client(clients[fd]->get_client_fd(), "USER SETTED\n");
		if (clients[fd]->get_nick() != ""){
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
			std::map<std::string, Channel *>::iterator it = this->channels.find(channelName);

			clients[clients[fd]->get_client_fd()]->addChannel(channelName, *channel);
			it->second->addUser(getClient(fd));

			_sendall(fd, clients[fd]->get_client_fd() + "JOIN #" + channelName + "\r\n");
			std::string topicMessage = ":server 332 " + clients[fd]->get_nick() + " " + channelName + " :Welcome to " + channelName + "\r\n";
			_sendall(fd, topicMessage);
			std::string anothermessage = ":server 353 " + clients[fd]->get_nick() + " = " + channelName + it->second->listAllUsers() + "\r\n";
			_sendall(fd, anothermessage);
			std::string anothermessage2 = ":server 353 " + clients[fd]->get_nick() + " " + channelName + "END OF NAMES\r\n";
			_sendall(fd, anothermessage2);
			_ToAll(channels[channelName], clients[fd]->get_client_fd(), "JOIN THIS I GUESS\n");
			/* std::string creationMessage = ":" + clients[fd]->get_mask() + " JOIN :#" + channelName + "\r\n";
			std::cout << creationMessage << std::endl;
			print_client(clients[fd]->get_client_fd(), creationMessage);

			clients[clients[fd]->get_client_fd()]->addChannel(it->first, *it->second);
			print_client(clients[fd]->get_client_fd(), "Channel " + channelName + " created and user added.\n"); */

			return ;
		}
		else{
			std::map<std::string, Channel *>::iterator it = this->channels.find(channelName);
			channels[channelName]->addUser(getClient(fd));
			std::string creationMessage = ":" + clients[fd]->get_mask() + " JOIN :#" + channelName + "\r\n";
			std::cout << creationMessage << std::endl;

			print_client(clients[fd]->get_client_fd(), creationMessage);
			clients[clients[fd]->get_client_fd()]->addChannel(channelName, *channel);

			std::string topicMessage = ":server 332 " + clients[fd]->get_nick() + " " + channelName + " :Welcome to " + channelName + "\r\n";
			_sendall(fd, topicMessage);
			std::string anothermessage = ":server 353 " + clients[fd]->get_nick() + " = " + channelName + it->second->listAllUsers() + "\r\n";
			_sendall(fd, anothermessage);
			std::string anothermessage2 = ":server 353 " + clients[fd]->get_nick() + " " + channelName + "END OF NAMES\r\n";
			_sendall(fd, anothermessage2);
			_ToAll(channels[channelName], clients[fd]->get_client_fd(), "JOIN THIS I GUESS\n");
			
			return ;
		}

        /* if (!channel) {
			createChannel(channelName);
            channel->addUser(...);
            std::string response = "Channel " + channelName + " created and user added.\n";
            send(client.get_client_fd(), response.c_str(), response.size(), 0);
        } */
	}	

	if (cmd == "privmsg" || cmd == "PRIVMSG"){
		//! change to varius types of privmsg
		std::string channel_name;
		iss >> channel_name;
		channel_name = channel_name.substr(1, channel_name.size() - 1);
		std::string msg;
		iss >> msg;

		std::map<std::string, Channel *>::iterator it = this->channels.find(channel_name);
		if (it != this->channels.end()){
			std::cout << "I DID ENTER IN FACT" << std::endl;
			std::cout << "PRIVMSG #" + channel_name + " " + msg << std::endl;
			_ToAll(it->second, clients[fd]->get_client_fd(), "PRIVMSG #" + channel_name + " " + msg + "\r\n");
		}

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

void server::_ToAll(Channel *channel, int ori_fd, std::string message){
	std::map<int, Client *> all_users = channel->getUsers();
	std::map<int, Client *>::iterator it = all_users.begin();
	std::string rep = this->clients[ori_fd]->get_mask();
	rep.append(message);
	while (it != all_users.end()){
		if (ori_fd != it->first)
			_sendall(it->first, message);
		it++;
	}
}


int server::_sendall(int destfd, std::string message)
{
	int total = 0;
	int bytesleft = message.length();
	int b;

	while (total < (int)message.length())
	{
		b = send(destfd, message.c_str() + total, bytesleft, 0);
		if (b == -1) break;
		total += b;
		bytesleft -= b;
	}
	return (b == -1 ? -1 : 0);
}

