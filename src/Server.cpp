#include "Server.hpp"
#include <errno.h>
#include "replies.hpp"


Server::Server(){}

void print_client(int client_fd, std::string str){
	send(client_fd, str.c_str(), str.size(), 0);
	std::cout << "[FOR DEBUG PURPOSES] Sent: " << str << "[DEBUG PURPOSES]" << std::endl;
}



Server::Server(int port, std::string pass) : _port(port), _pass(pass), _nfds(1), _cur_online(1){
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

Server::~Server() {
	close(_socket_Server);
	close(_epoll_fd);
	for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++) {
		close(it->first);
	}
	for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); it++) {
		delete it->second;
	}
	this->clients.clear();
}

int const &Server::get_socket() const{
	return _socket_Server;
}

void Server::binding(){
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(_port);
    bind(_socket_Server, (const struct sockaddr *)&_addr, sizeof(_addr));
    listen(_socket_Server, 10);

    _events[0].data.fd = _socket_Server;
    _events[0].events = EPOLLIN;

    if(epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _socket_Server, &_events[0]) == -1){
        std::cerr << "Error adding socket to epoll: " << strerror(errno) << std::endl;
        close(_socket_Server);
        exit(EXIT_FAILURE);
    }
}

void Server::loop(){
    while(true){
        // std::cout << "Fase 0: " << _cur_online << " " << _nfds << std::endl;
        _nfds = epoll_wait(_epoll_fd, _events, 10, -1);
        // std::cout << "Fase 0: " << _cur_online << " " << _nfds << std::endl;
        if (_nfds == -1) {
            std::cerr << "Error during epoll_wait: " << strerror(errno) << std::endl;
            // exit(EXIT_FAILURE);
        }
        for(int i = 0; i < _nfds; i++){
            // std::cout << "Fase 1\n";
            if (_events[i].events & EPOLLIN) {
                if(_events[i].data.fd == _socket_Server){
                    funct_NewClient();
                } else {
                    funct_NotNewClient(i);
                }
            }
            // std::cout << "END Fase 1\n";
        }
        // std::cout << "END Fase 0: " << _cur_online << std::endl;
    }
}

void Server::handleCommands(int fd, const std::string &command){
	std::istringstream iss(command);
	std::string cmd;
	size_t pos;
	iss >> cmd;
	if (cmd == "auth" || cmd == "AUTH"){
		std::cout << "PASS SERVER: " << _pass << std::endl;
		std::cout << "GET PASS: " << clients[fd]->get_pass() << std::endl;
		std::string pass = clients[fd]->get_pass();
		/* for(size_t i = 0; pass[i] != '\0'; i++){
			std::cout << "123>" << pass[i] << "<<<<<<" << std::endl;} */
		std::cout << strcmp(pass.c_str(), _pass.c_str()) << std::endl;
		if (strcmp(pass.c_str(), _pass.c_str()) == 0){
			print_client(clients[fd]->get_client_fd(), ":server 371 " + clients[fd]->get_nick() +": User is Authenticated\n");
			clients[fd]->set_auth(true);
			return ;
		}
		else{
			print_client(clients[fd]->get_client_fd(), ":server 464 " + clients[fd]->get_nick() + " :Password incorrect\n");
			print_client(clients[fd]->get_client_fd(), ":server 451 " + clients[fd]->get_nick() + " :You have not registered\n");
			return ;
		}

	}
		// Procurar por USER
	pos = command.find("USER");
	if (pos != std::string::npos) {
		// Extrair a informação do usuário e armazená-la no vetor
		std::string line = command.substr(pos);
		this->clients[fd]->set_name(extract_value(line, "USER"));
		// std::cout << "start>>" << this->clients[fd]->get_name() << "<<end\n" << std::endl;
	}

	// Procurar por PASS
	pos = command.find("PASS");
	if (pos != std::string::npos) {
		// Extrair a informação da senha e armazená-la no vetor
		std::string line = command.substr(pos);
		this->clients[fd]->set_pass(extract_value(line, "PASS"));
		// std::cout << "start>>" << this->clients[fd]->get_pass() << "<<end\n" << std::endl;
	}

	// Procurar por NICK
	pos = command.find("NICK");
	if (pos != std::string::npos) {
		// Extrair a informação do apelido e armazená-la no vetor
		std::string line = command.substr(pos);
		if (this->clients[fd]->get_nick().empty()) {
			this->clients[fd]->set_nick(extract_value(line, "NICK"));
			std::string response = ":" + clients[fd]->get_nick() + "!" + clients[fd]->get_name() + "@" + clients[fd]->get_host() + " ";
			this->clients[fd]->set_mask(response);
		}
		else{
			std::string Newnick = extract_value(line, "NICK");
			std::string changeNick = ":" + this->clients[fd]->get_nick() + " NICK " + Newnick + "\r\n";
			print_client(clients[fd]->get_client_fd(), changeNick);
			this->clients[fd]->set_nick(Newnick);
			std::string response = ":" + clients[fd]->get_nick() + "!" + clients[fd]->get_name() + "@" + clients[fd]->get_host() + " ";
			this->clients[fd]->set_mask(response);
		}

		// std::cout << "start>>" << this->clients[fd]->get_nick() << "<<end\n" << std::endl;
	}

	if (cmd == "join" || cmd == "JOIN"){
		if (clients[fd]->get_auth() == false){
			print_client(clients[fd]->get_client_fd(), "Need to Auth the user\n");
			print_client(clients[fd]->get_client_fd(), ERR_NOSUCHNICK(clients[fd]->get_nick()));
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
		std::string creationMessage;
		if (channel == NULL){
			std::cout << "Channel already exists\n";
			createChannel(channelName);
			channels[channelName]->addUser(getClient(fd));
			creationMessage = clients[fd]->get_mask() + "JOIN :" + channelName + "\n";
			std::cout << creationMessage << std::endl;
			print_client(clients[fd]->get_client_fd(), creationMessage);
			// print_client(clients[fd]->get_client_fd(), "Channel " + channelName + " created and user added.\r\n");

			// Send topic message
			// After JOIN handling
			std::string topicMessage = RPL_NOTOPIC(channelName);
			print_client(clients[fd]->get_client_fd(), topicMessage);
			
			// Send RPL_NAMREPLY (353) and RPL_ENDOFNAMES (366) afterwards
			std::string namesMessage = ":server 353 " + clients[fd]->get_nick() + " = " + channelName + " :" + clients[fd]->get_nick() + "\r\n";
			print_client(clients[fd]->get_client_fd(), namesMessage);
			
			std::string endOfNamesMessage = ":server 366 " + clients[fd]->get_nick() + " " + channelName + " :End of /NAMES list\r\n";
			print_client(clients[fd]->get_client_fd(), endOfNamesMessage);
			std::string response = ":" + clients[fd]->get_nick() + "!" + clients[fd]->get_name() + "@" + clients[fd]->get_host() + " JOIN " + channelName + "\n";
			// _ToAll(channels[channelName], clients[fd]->get_client_fd(), response);
			broadcast_to_channel(channelName, clients[fd]->get_client_fd());
		}
		else {
			std::cout << "Channel already exists\n";
			channels[channelName]->addUser(getClient(fd));
			creationMessage = clients[fd]->get_mask() + "JOIN :" + channelName + "\n";
			print_client(clients[fd]->get_client_fd(), creationMessage);
			std::string reply = "JOIN " + channelName + "\n";
			// _ToAll(channels[channelName], clients[fd]->get_client_fd(), reply);
			broadcast_to_channel(channelName, clients[fd]->get_client_fd());
			// for (int i = 0; i < _cur_online; i++) {
			// 		print_client(_events[i].data.fd, response);
			// }
			std::string namesMessage = ":server 353 " + clients[fd]->get_nick() + " = " + channelName + " :" + clients[fd]->get_nick() + "\r\n";
			print_client(clients[fd]->get_client_fd(), namesMessage);
			
			std::string endOfNamesMessage = ":server 366 " + clients[fd]->get_nick() + " " + channelName + " :End of /NAMES list\r\n";
			print_client(clients[fd]->get_client_fd(), endOfNamesMessage);
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

void Server::createChannel(const std::string &channelName){
	if (channels.find(channelName) == channels.end()){
		Channel *channel = new Channel(channelName);
		channels.insert(std::pair<std::string, Channel *>(channelName, channel));
	}
}

Channel *Server::getChannel(const std::string name)  {
	if (channels.find(name) != channels.end())
		return channels[name];
	return NULL;
}


Client &Server::getClient(int fd){
	std::map<int, Client *>::iterator it = clients.find(fd);
	return *it->second;
}
/* 
void Server::setUsers(std::string userName){
	std::vector<std::string>::iterator it = user.  
	if (user.(userName) == user.end())
		user.insert(user);
}

std::string const &Server::getUser()const{

} */

void Server::funct_NewClient(){
    struct sockaddr_storage remote_addr;
    socklen_t client_len = sizeof(remote_addr);
    // std::cout << "Fase 2\n";
    int newsocket = accept(_socket_Server, (struct sockaddr*)&remote_addr, &client_len);
    if (newsocket == -1) {
        std::cerr << "Error accepting new connection: " << strerror(errno) << std::endl;
    } else {
        _events[_cur_online].events = EPOLLIN;
        _events[_cur_online].data.fd = newsocket;
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, newsocket, &_events[_cur_online]) == -1) {
            std::cerr << "Error adding new socket to epoll: " << strerror(errno) << std::endl;
            close(newsocket);
        } else {
            this->clients.insert(std::pair<int, Client *>(newsocket, new Client(newsocket)));
            this->_cur_online++;
        }
    }
}

void Server::funct_NotNewClient(int i){
    // std::cout << "Fase 3\n";
    char buf[6000];
    int sender_fd = _events[i].data.fd;
    int bytes_received = recv(sender_fd, buf, sizeof(buf), 0);
    
    if (bytes_received <= 0){
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, sender_fd, NULL) == -1) {
            std::cerr << "Error removing socket from epoll: " << strerror(errno) << std::endl;
        }
        close(sender_fd);
        this->clients.erase(sender_fd);
        this->_cur_online--;
    } else {
        std::string command(buf, bytes_received);
        if (!command.empty() && command[command.size() - 1] == '\r') {
            command.erase(command.end() - 1);
        }
        handleCommands(sender_fd, command);
        std::cout << "Calling handleCommands with fd: " << sender_fd << " and command: " << command << std::endl;
    }
    // std::cout << "END Fase 3\n";
}

std::string Server::extract_value(const std::string& line, const std::string& key) {
	size_t start = line.find(key) + key.length();  // Find end of key
	if (start == std::string::npos) {
		return "";  // Key not found
	}
	// Skip any spaces or colons that follow the key
	while (start < line.length() && (line[start] == ' ' || line[start] == ':')) {
		start++;
	}
	// Find the end of the first word (next space or newline)
	size_t end = line.find_first_of(" \r\n", start);
	if (end == std::string::npos) {
		end = line.length();  // If no space or newline, take until the end of the line
	}
	std::string value = line.substr(start, end - start);

	// Remove any trailing newline character
	if (!value.empty() && value[value.length() - 1] == '\n') {
		value = value.substr(0, value.length() - 1);
	}

	return value;
}

void Server::broadcast_to_channel(const std::string &channelName, int sender_fd) {
    // Check if the channel exists
    if (channels.find(channelName) != channels.end()) {
        Channel* channel = channels[channelName];
        
        // Iterate through all users in the channel
        std::map<int, Client*>::iterator it;
        for (it = channel->getUsers().begin(); it != channel->getUsers().end(); ++it) {
            int client_fd = it->first;
			std::string response = ":" + clients[sender_fd]->get_nick() + "!" + clients[sender_fd]->get_name() + "@" + clients[sender_fd]->get_host() + " JOIN :" + channelName + "\r\n";
            if (client_fd != sender_fd) {
                print_client(client_fd, response);
            }
			else{
				std::map<int, Client*>::iterator it2;
				for (it2 = channel->getUsers().begin(); it2 != channel->getUsers().end(); ++it2){
					int client_fd2 = it2->first;
					std::string response = ":" + clients[it2->first]->get_nick() + "!" + clients[it2->first]->get_name() + "@" + clients[it2->first]->get_host() + " JOIN :" + channelName + "\r\n";
					if (client_fd2 != sender_fd)
						print_client(sender_fd, response);
				}
			}
        }
    }
}

// void Server::_ToAll(Channel *channel, int ori_fd, std::string message){
// 	std::map<int, Client *> all_users = channel->getUsers();
// 	std::map<int, Client *>::iterator it = all_users.begin();
// 	std::string rep = this->clients[ori_fd]->get_mask();
// 	rep.append(message);
// 	std::cout << "Original FD: " << ori_fd << std::endl;
// 	while (it != all_users.end()){
// 		if (ori_fd != it->first){
// 			print_client(it->first, rep);
// 		}
// 		it++;
// 	}
// }
