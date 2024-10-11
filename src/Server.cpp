#include "Server.hpp"

Server::Server(){}

Server::Server(int port, std::string pass) : _port(port), _pass(pass), _nfds(1), _cur_online(0) {
	//! DONT KNOW WHERE TO PUT THIS
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1) {
		std::cerr << "Error creating epoll file descriptor" << std::endl;
		exit(EXIT_FAILURE);
	}
}

Server::~Server() {
	close(_socket_Server);
	close(_epoll_fd);
	for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++) {
		delete it->second;
	}
	clients.clear();

	for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); ++it) {
		delete it->second;
	}
	channels.clear();
}

int const &Server::get_socket() const{
	return _socket_Server;
}

void Server::binding(){
	struct addrinfo hints, *serverinfo, *tmp;
	int status;

	memset(&hints, 0, sizeof(hints));
	memset(&_events, 0, sizeof(_events));
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
		this->_socket_Server = socket(tmp->ai_family, tmp->ai_socktype | O_NONBLOCK, tmp->ai_protocol);
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
	_events[0].events = EPOLLIN | EPOLLET;
	
	if(epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _socket_Server, _events) == -1){
		std::cerr << "Error adding socket to epoll" << std::endl;
		close(_socket_Server);
		exit(EXIT_FAILURE);
	}
	_cur_online++;
}

void Server::loop(){
	while(true){
		std::cout << "Waiting for connections..." << std::endl;
		_nfds = epoll_wait(_epoll_fd, _events, 10, -1);

		if (_nfds == -1) {
			std::cerr << "Error during epoll_wait: " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}
		for(int i = 0; i < _cur_online; i++){
			if (_events[i].events & EPOLLIN) {
				if(_events[i].data.fd == _socket_Server){
					funct_NewClient(i);
				} else {
					funct_NotNewClient(i);
				}
			}
		}
	}
}

void Server::funct_NewClient(int i){
	struct sockaddr_storage client_addr;
	socklen_t client_len = sizeof(client_addr);
	int newsocket = accept(_socket_Server, (struct sockaddr*)&client_addr, &client_len);
	if (newsocket == -1) {
		std::cerr << "Error accepting new connection: " << strerror(errno) << std::endl;
	}
	fcntl(newsocket, F_SETFL, O_NONBLOCK);

	_events[i].data.fd = newsocket;
	_events[i].events = EPOLLIN | EPOLLET;
	this->clients.insert(std::pair<int, Client *>(newsocket, new Client(newsocket)));
	clients[newsocket]->set_addr(client_addr);

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, newsocket, &_events[i]) == -1) {
		std::cerr << "Error adding new socket to epoll: " << strerror(errno) << std::endl;
		close(newsocket);
	}
	this->_cur_online++;
}

void Server::funct_NotNewClient(int i){
	this->clients[_events[i].data.fd]->bytes_received += recv(_events[i].data.fd, this->clients[_events[i].data.fd]->_buffer + clients[_events[i].data.fd]->bytes_received, sizeof(this->clients[_events[i].data.fd]->_buffer) - clients[_events[i].data.fd]->bytes_received, 0);
	if (this->clients[_events[i].data.fd]->bytes_received == 0) {
    // Client disconnected
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _events[i].data.fd, NULL) == -1) {
			std::cerr << "Error removing socket from epoll(not new client): " << strerror(errno) << std::endl;
		} else {
			close(_events[i].data.fd);
			std::cerr << _RED << "Client disconnected. Current onlines: " << _cur_online << _END << std::endl;
			this->clients.erase(_events[i].data.fd);
			this->_cur_online--;
		}
	} 
	else if (this->clients[_events[i].data.fd]->bytes_received == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			// Real error, remove the client
			if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _events[i].data.fd, NULL) == -1) {
				std::cerr << "Error removing socket from epoll(not new client): " << strerror(errno) << std::endl;
			} else {
				close(_events[i].data.fd);
				std::cerr << _RED << "Error in recv(). Current onlines: " << _cur_online << _END << std::endl;
				this->clients.erase(_events[i].data.fd);
				this->_cur_online--;
			}
		} 
		else {
			// EAGAIN/EWOULDBLOCK: temporary non-fatal error, do nothing
			std::cerr << _YELLOW << "Temporary recv() error: " << strerror(errno) << _END << std::endl;
		}
	} 
	else {
		int extra_bytes = 0;
		while (this->clients[_events[i].data.fd]->_buffer[this->clients[_events[i].data.fd]->bytes_received - 1] != '\n'){
			extra_bytes = recv(_events[i].data.fd, this->clients[_events[i].data.fd]->_buffer + this->clients[_events[i].data.fd]->bytes_received, sizeof(this->clients[_events[i].data.fd]->_buffer) - this->clients[_events[i].data.fd]->bytes_received, 0);
			if (extra_bytes == -1){
				if (errno == EAGAIN && errno == EWOULDBLOCK) {
					//! some debug message
					return;
				}
				else{
					//! actual error message
					return;
				}
			}
			if (extra_bytes > 0)
				clients[_events[i].data.fd]->bytes_received += extra_bytes;
			else if(extra_bytes == 0){
					return;
				//! error message
			}
		}
		// Successfully received data
		this->clients[_events[i].data.fd]->_buffer[this->clients[_events[i].data.fd]->bytes_received] = '\0';
			std::string command(this->clients[_events[i].data.fd]->_buffer);
			if (!command.empty() && command[command.size() - 1] == '\r') {
				command.erase(command.end() - 1);
			}
			handleCommands(_events[i].data.fd, command);
			std::cout << _RED << "COMMAND SENT BY CLIENT: " << _events[i].data.fd << " " << _END << _GREEN << command << _RED << "END OF COMMAND" << _END << std::endl;
		}
	this->clients[_events[i].data.fd]->bytes_received = 0;
	memset(this->clients[_events[i].data.fd]->_buffer, 0, 1024);
}


// std::vector<std::string> Server::parser(const std::string &command){
// 	 std::vector<std::string> result;
//     std::stringstream ss(command);
//     std::string item;
    
//     while (std::getline(ss, item, ' ')) {
//         result.push_back(item);
//     }
// 	return result;
// }




//! VERIFY AMOUNT OF ARGUMENTS PASS TO THE COMMANDS
void Server::handleCommands(int fd, const std::string &command){
	// //TODO: CHANGE WAY TO RECEIVE COMMANDS
	// std::vector<std::string> args = parser(command);
	// for (std::size_t i = 0; i < args.size(); ++i) {
    //     std::cout << "VECTOR ARGS: " << args[i] << std::endl;
    // }

	std::istringstream commandStream(command);
    std::string line;
    // TODO: Percorre cada linha do comando
    while (std::getline(commandStream, line, '\n')) {
        std::istringstream iss(line);
        std::string cmd;
		iss >> cmd;
		std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
		if (cmd == "AUTH")
			handleAuth(fd);
		else if (cmd == "PASS")
			handlePass(fd, iss);
		else if (cmd == "NICK")
			handleNick(fd, iss);
		else if (cmd == "USER")
			handleUser(fd, iss);
		else if (cmd == "JOIN")
			handleJoin(fd, iss);
		else if (cmd == "PRIVMSG")
			handlePrivmsg(fd, iss);
		else if (cmd == "PART")
			handlePart(fd, iss);
		else if (cmd == "QUIT")
			handleQuit(fd, iss);
		else if (cmd == "PING")
			handlePing(fd, iss);
		else if (cmd == "MODE")
			handleMode(fd, iss);
		else if (cmd == "KICK")
			handleKick(fd, iss);
		else if (cmd == "INVITE")
			handleInvite(fd, iss);
		else if (cmd == "TOPIC")
			handleTopic(fd, iss);
		else if (cmd == "WHO")
			handleWho(fd, iss);
		else if (cmd == "WHOIS")
			handleWhois(fd, iss);
		else if (cmd == "LIST")
			handleList(fd);
	}
}

void Server::createChannel(const std::string &channelName, int fd){
	std::map<std::string, Channel *>::iterator it = channels.find(channelName);
	if (it == channels.end()){
		Channel *channel = new Channel(channelName, this->clients[fd]);
		channels.insert(std::pair<std::string, Channel *>(channelName, channel));
		channels[channelName]->addModes('n');
		channels[channelName]->addModes('t');
	}
/* 	else{
		if (this->clients[fd]->get_isOperator() == true)
			it->second->addOperator(getClient(fd));
		else
			it->second->addUser(getClient(fd));
	} */
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

std::string Server::extract_value(const std::string& line) {
	size_t start = 0;  // Find end of key
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

void Server::_ToAll(int ori_fd, std::string message){
	std::set<std::string> channelList = findInChannel(ori_fd);
	while (!channelList.empty()){
		std::string channelName = *channelList.begin();
		std::map<int, Client *> all_users = channels[channelName]->getUsers();
		std::map<int, Client *>::iterator it = all_users.begin();
		while (it != all_users.end()){
			if (ori_fd != it->first){
				print_client(it->first, message);
			}
			it++;
		}
		channelList.erase(channelList.begin());
	}
}

void Server::_ToAll(Channel *channel, int ori_fd, std::string message){
	std::map<int, Client *> all_users = channel->getUsers();
	std::map<int, Client *>::iterator it = all_users.begin();
	std::string rep = this->clients[ori_fd]->get_mask();
	rep.append(message);
	while (it != all_users.end()){
		if (ori_fd != it->first)
			print_client(it->first, rep);
		it++;
	}
}

// Essa tem como objetivo localizar o usuário em um canal dentre todos os canais armazendaos no servidor
// em caso positivo, retorna a lista com o nome dos canais, caso contrário, retorna null.
std::set<std::string> Server::findInChannel(int fd){
	std::set<std::string> channelList;
	std::map<std::string, Channel *>::iterator it = channels.begin();
	while (it != channels.end()){
		std::map<int, Client *> users = it->second->getUsers();
		if (users.find(fd) != users.end()) {
			channelList.insert(it->first);
		}
		it++;
	}
	return channelList;
}

void Server::print_client(int client_fd, std::string str){
	send(client_fd, str.c_str(), str.size(), 0);
	// std::cout << "[FOR DEBUG PURPOSES] Sent: " << str << "[DEBUG PURPOSES]" << std::endl;
}

void Server::sendCode(int fd, std::string num, std::string nickname, std::string message){
	if (nickname.empty())
		nickname = "*";
	print_client(fd, ":server " + num + " " + nickname + " " + message + "\r\n");
}

int Server::_sendall(int destfd, std::string message)
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

bool Server::findNick(std::string nick){
	for(std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++){
		if (it->second->get_nick() == nick)
			return true;
	}
	return false;
}

// void server::setBuffer(std::string buffer)
// {
// 	this->_buffer.append(buffer);
// }
