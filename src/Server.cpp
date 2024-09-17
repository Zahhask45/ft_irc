#include "Server.hpp"
#include <errno.h>
#include "replies.hpp"

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
	for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); it++) {
		delete it->second;
	}
	this->clients.clear();
	this->channels.clear();
}

int const &Server::get_socket() const{
	return _socket_Server;
}

void Server::binding(){
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
		std::cout << "Number of clients: " << _cur_online << std::endl;
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
	_events[i].events = EPOLLIN;
	this->clients.insert(std::pair<int, Client *>(newsocket, new Client(newsocket)));
	clients[newsocket]->set_addr(client_addr);

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, newsocket, &_events[i]) == -1) {
		std::cerr << "Error adding new socket to epoll: " << strerror(errno) << std::endl;
		close(newsocket);
	}
	this->_cur_online++;
}

void Server::funct_NotNewClient(int i){
	int bytes_received = recv(_events[i].data.fd, _buffer, sizeof(_buffer), 0);
	if (bytes_received <= 0){
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _events[i].data.fd, NULL) == -1) {
			std::cerr << "Error removing socket from epoll: " << strerror(errno) << std::endl;
		}
		close(_events[i].data.fd);
        this->clients.erase(_events[i].data.fd);
        this->_cur_online--;
	}
	else {
		_buffer[bytes_received] = '\0';
		std::string command(_buffer);
		if (!command.empty() && command[command.size() - 1] == '\r') {
			command.erase(command.end() - 1);
		}
		handleCommands(_events[i].data.fd, command);
		std::cout << "Calling handleCommands with fd: " << _events[i].data.fd << " and command: " << command << std::endl;
	}
	memset(_buffer, 0, 1024);
}

//! VERIFY AMOUNT OF ARGUMENTS PASS TO THE COMMANDS
void Server::handleCommands(int fd, const std::string &command){
	std::istringstream commandStream(command);
    std::string line;
    
    // Percorre cada linha do comando
    while (std::getline(commandStream, line, '\n')) {
        std::istringstream iss(line);
        std::string cmd;
		iss >> cmd;
		// std::cout << "[Command]: " << cmd << " [Command]" << std::endl;
		if (cmd == "auth" || cmd == "AUTH"){
			std::cout << "PASS SERVER: " << _pass << std::endl;
			std::cout << "GET PASS: " << clients[fd]->get_pass() << std::endl;
			std::string pass = clients[fd]->get_pass();
			std::cout << strcmp(pass.c_str(), _pass.c_str()) << std::endl;
			if (strcmp(pass.c_str(), _pass.c_str()) == 0){
				sendCode(fd, "371", clients[fd]->get_nick(), ": User is Authenticated\n");
				clients[fd]->set_auth(true);
				return ;
			}
			else{
				sendCode(fd, "464", clients[fd]->get_nick(), ": Password incorrect\n");
				sendCode(fd, "451", clients[fd]->get_nick(), ": You have not registered\n");
				return ;
			}
		}
		if (cmd == "pass" || cmd == "PASS"){
			std::string pass;
			iss >> pass;
			pass = extract_value(pass, "PASS");
			if (_pass != pass){
				print_client(fd, "WRONG PASSWORD\n");
				return ;
			}
			clients[fd]->set_pass(pass);
			print_client(fd, "PASSWORD CONFIRMED\n");
		}

		if (cmd == "nick" || cmd == "NICK"){
			// Extrair a informação do apelido e armazená-la no vetor
			std::string nick;
			iss >> nick;
			if (this->clients[fd]->get_nick().empty()) {
				this->clients[fd]->set_nick(nick);
				std::string response = ":" + clients[fd]->get_nick() + "!" + clients[fd]->get_user() + "@" + clients[fd]->get_host() + " ";
				this->clients[fd]->set_mask(response);
			}
			else{
				std::string changeNick = ":" + this->clients[fd]->get_nick() + " NICK " + nick + "\r\n";
				std::string nickChangeMsg = ":" + this->clients[fd]->get_nick() + "!" + clients[fd]->get_user() + "@" + clients[fd]->get_host() + " NICK :" + nick + "\r\n";
				print_client(fd, changeNick);
				this->clients[fd]->set_nick(nick);
				std::string response = ":" + clients[fd]->get_nick() + "!" + clients[fd]->get_user() + "@" + clients[fd]->get_host() + " ";
				this->clients[fd]->set_mask(response);
				_ToAll(fd, nickChangeMsg);
			}
		}
		if (cmd == "user" || cmd == "USER") {
			// Extrair a informação do usuário e armazená-la no vetor
			std::string user;
			iss >> user;
			this->clients[fd]->set_user(extract_value(user, "USER"));
			if (clients[fd]->get_nick() != "")
				clients[fd]->set_mask(":" + clients[fd]->get_nick() + "!" + clients[fd]->get_user() + "@" + clients[fd]->get_host() + " ");
		}

		if (cmd == "join" || cmd == "JOIN"){
			if (clients[fd]->get_auth() == false){
				print_client(fd, "Need to Auth the user\n");
				print_client(fd, ERR_NOSUCHNICK(clients[fd]->get_nick()));
				return ;
			}
			std::string channelName;
			iss >> channelName;
			if (channelName.empty()){
				print_client(fd, "Channel name is empty\n");
				return ;
			}
			if (getChannel(channelName) == NULL)
				createChannel(channelName);
			clients[fd]->addChannel(channelName, *channels[channelName]);
			channels[channelName]->addUser(getClient(fd));
			std::string creationMessage = clients[fd]->get_mask() + "JOIN :" + channelName + "\n";
			print_client(fd, creationMessage);

			// std::string topicMessage = RPL_NOTOPIC(clients[fd]->get_nick(), channelName);
			std::string topicMessage = ":server 332 " + clients[fd]->get_mask() + " " + channelName + " :Welcome to " + channelName + "\r\n";
			print_client(fd, topicMessage);

			std::string namesMessage = ":server 353 " + clients[fd]->get_nick() + " = " + channelName + " :" + clients[fd]->get_nick() + "\r\n";
			print_client(fd, namesMessage);

			std::string endOfNamesMessage = ":server 366 " + clients[fd]->get_nick() + " " + channelName + " :End of /NAMES list\r\n";
			print_client(fd, endOfNamesMessage);

			broadcast_to_channel(channelName, fd);
		}
		if (cmd == "privmsg" || cmd == "PRIVMSG"){
			//! change to varius types of privmsg
			std::string channel_name;
			iss >> channel_name;
			//channel_name = channel_name.substr(1, channel_name.size() - 1);
			size_t pos;
			pos = command.find(channel_name);
			if (pos == std::string::npos) {
				return;
			}
			//! NEED TO PUT THIS BETTER
			std::string msg = command.substr(pos + channel_name.size() + 1, command.size() - (pos + channel_name.size() + 1));
			// std::cout << "start>>" << msg << "<<end\n" << std::endl;

			std::map<std::string, Channel *>::iterator it = this->channels.find(channel_name);
			if (it != this->channels.end()){
				// std::cout << "INSIDE THE PRIVMSG >>>> " << "PRIVMSG " + channel_name + " " + msg << std::endl;
				_ToAll(it->second, fd, "PRIVMSG " + channel_name + " " + msg + "\n");
			}
		}
		if (cmd == "part" || cmd == "PART"){
			std::string channelName;
			iss >> channelName;
			if (channelName.empty()){
				print_client(fd, "Channel name is empty\n");
				return ;
			}
			if (channels.find(channelName) == channels.end()){
				print_client(fd, "Channel does not exist\n");
				return ;
			}
			Channel *channel = channels[channelName];
			if (channel->getUsers().find(fd) == channel->getUsers().end()){
				print_client(fd, "You are not in this channel\n");
				return ;
			}
			std::string response = ":" + clients[fd]->get_nick() + "!" + clients[fd]->get_user() + "@" + clients[fd]->get_host() + " PART " + channelName + "\r\n";
			std::string response2 = clients[fd]->get_mask() + " PART " + channelName + "\r\n";
			print_client(fd, response);
			_ToAll(channel, fd, "PART " + channelName + "\r\n");
			channel->removeUser(clients[fd]->get_nick());
			clients[fd]->removeChannel(channelName);
		}
		if (cmd == "quit" || cmd == "QUIT"){
			std::string message;
			iss >> message;
			std::string response = ":" + clients[fd]->get_nick() + "!" + clients[fd]->get_user() + "@" + clients[fd]->get_host() + " QUIT :" + message + "\r\n";
			_ToAll(fd, response);
			print_client(fd, response);
		}
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

std::string Server::extract_value(const std::string& line, const std::string& key) {
	(void)key;
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

void Server::broadcast_to_channel(const std::string &channelName, int last_fd) {
	// Check if the channel exists
	if (channels.find(channelName) != channels.end()) {
		Channel* channel = channels[channelName];

		// Iterate through all users in the channel
		std::map<int, Client*>::iterator it;
		for (it = channel->getUsers().begin(); it != channel->getUsers().end(); ++it) {
			int client_fd = it->first;
			std::string response = ":" + clients[last_fd]->get_nick() + "!" + clients[last_fd]->get_user() + "@" + clients[last_fd]->get_host() + " JOIN :" + channelName + "\r\n";
			std::string response2 = ":" + clients[client_fd]->get_nick() + "!" + clients[client_fd]->get_user() + "@" + clients[client_fd]->get_host() + " JOIN :" + channelName + "\r\n";

			if (client_fd != last_fd) {
				print_client(client_fd, response);
				print_client(last_fd, response2);
			}
		}
	}
}

//Essa funcao recebe o fd e a mensagem a ser enviada para todos os clientes conectados no mesmo canal que o usuario do fd.
// é auxiliada pela função findInChannel que retorna o nome do canal em que o usuário está conectado.
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
	std::cout << "[FOR DEBUG PURPOSES] Sent: " << str << "[DEBUG PURPOSES]" << std::endl;
}

void Server::sendCode(int fd, std::string num, std::string nickname, std::string message){
	if (nickname.empty())
		nickname = "*";
	print_client(fd, ":server " + num + " " + nickname + " " + message + "\n");
}
