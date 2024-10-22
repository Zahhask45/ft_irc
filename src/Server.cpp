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

	int opt = 1;
	status = getaddrinfo("0.0.0.0", toString(_port).c_str(), &hints, &serverinfo);
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
		std::cerr << "Failed to bind to any address..." << std::endl;
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
		_nfds = epoll_wait(_epoll_fd, _events, 100, -1);

		if (_nfds == -1) {
			std::cerr << "Error during epoll_wait: " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}
		for(int i = 0; i < _cur_online; i++){
			if (_events[i].events & EPOLLIN) {
				if(_events[i].data.fd == _socket_Server)
					funct_new_client(i); 
				else
					funct_not_new_client(i);
			}
		}
	}
}

void Server::funct_new_client(int i){
	struct sockaddr_storage client_addr;
	socklen_t client_len = sizeof(client_addr);
	int newsocket = accept(_socket_Server, (struct sockaddr*)&client_addr, &client_len);
	if (newsocket == -1)
		std::cerr << "Error accepting new connection: " << strerror(errno) << std::endl;
	fcntl(newsocket, F_SETFL, O_NONBLOCK);

	_events[i].data.fd = newsocket;
	_events[i].events = EPOLLIN;
	this->clients.insert(std::make_pair(newsocket, new Client(newsocket)));
	clients[newsocket]->set_addr(client_addr);

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, newsocket, &_events[i]) == -1) {
		std::cerr << "Error adding new socket to epoll: " << strerror(errno) << std::endl;
		close(newsocket);
	}
	this->_cur_online++;
}

void Server::funct_not_new_client(int i){
	int extra_bytes = 0;
	char buffer_ptr[1024];
	extra_bytes = recv(_events[i].data.fd,  buffer_ptr, sizeof(buffer_ptr), 0);
	std::cout << _PURPLE << "INFO RECEIVED: ";
	for (int ii = 0 ; ii < extra_bytes; ii++)
	{
		std::cout << buffer_ptr[ii];
	}
	std::cout << std::endl << "AND EXTRA_BYTES: " << extra_bytes << std::endl;
	std::cout << std::endl << "RECEIVED BY CLIENT: " << _events[i].data.fd << _END << std::endl;
	if (extra_bytes == -1)
	{
		int err_code = 0;
    	socklen_t len = sizeof(err_code);
		if (getsockopt(_events[i].data.fd, SOL_SOCKET, SO_ERROR, &err_code, &len) == -1){
        	std::cout << _RED << "Error on getsocket()" << _END << std::endl;
			if (this->clients.find(_events[i].data.fd) == this->clients.end()) {
				return ;
			}
			throw std::invalid_argument("SOME ERROR");
		}
		else if(err_code == EAGAIN || err_code == EWOULDBLOCK)
				std::cout << _YELLOW << "Temporary recv() error" << _END << std::endl;
        else if(err_code != 0){

            errno = err_code;
			if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _events[i].data.fd, NULL) == -1) {
				std::cerr << "Error removing socket from epoll(not new client): " << strerror(errno) << std::endl;
				return;
			}
			else
			{
				close(_events[i].data.fd);
				std::cerr << _RED << "Error in recv(). Current onlines: " << _cur_online  << " WITH ERROR NO: " << err_code << _END << std::endl;
				this->clients.erase(_events[i].data.fd);
				this->_cur_online--;
			}
		}
	}
	if (extra_bytes > 0){
		_events[i].events = EPOLLOUT;
		clients[_events[i].data.fd]->set_bytes_received(clients[_events[i].data.fd]->get_bytes_received() + extra_bytes);
		buffer_ptr[extra_bytes] = '\0';
		std::cout << buffer_ptr << std::endl;
		this->clients[_events[i].data.fd]->add_to_buffer(buffer_ptr);
	}
	else if (extra_bytes == 0)
	{
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _events[i].data.fd, NULL) == -1)
		{
			std::cerr << "Error removing socket from epoll(not new client): " << strerror(errno) << std::endl;
		}
		else
		{
			close(_events[i].data.fd);
			std::cerr << _RED << "Client disconnected (FROM THE NEW STUFF). Current onlines: " << _cur_online << _END << std::endl;
			this->clients.erase(_events[i].data.fd);
			this->_cur_online--;
		}
		return;
	}
	if (this->clients.find(_events[i].data.fd) == this->clients.end() && this->clients[_events[i].data.fd] != 0)
		return;
	std::string command(this->clients[_events[i].data.fd]->get_buffer());
	if (this->clients[_events[i].data.fd]->get_buffer().find("\n") == std::string::npos){
		std::cout << ">>" << this->clients[_events[i].data.fd]->get_buffer() << "<<" << std::endl;	
		return;
	}
	if (!command.empty() && command[command.size() - 1] == '\r')
	{
		command.erase(command.end() - 1);
	}
	handle_commands(_events[i].data.fd, command);
	memset(buffer_ptr, 0, 1024);
	std::cout << _RED << "COMMAND SENT BY CLIENT: " << _events[i].data.fd << " " << _END << _GREEN << command << _RED << "END OF COMMAND" << _END << std::endl;
	if (this->clients.find(_events[i].data.fd) != this->clients.end() && this->clients[_events[i].data.fd] != 0) { // The solution i found.
		this->clients[_events[i].data.fd]->set_bytes_received(0); //! this is making the server SEGFAULT when the client disconnects.
		this->clients[_events[i].data.fd]->clean_buffer();
		_events[i].events = EPOLLIN;

	}
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
void Server::handle_commands(int fd, const std::string &command){

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

void Server::create_channel(const std::string &channelName, int fd){
	std::map<std::string, Channel *>::iterator it = channels.find(channelName);
	if (it == channels.end()){
		Channel *channel = new Channel(channelName, this->clients[fd]);
		channels.insert(std::make_pair(channelName, channel));
		channels[channelName]->add_modes('t');
	}
}

Channel *Server::get_channel(const std::string name)  {
	if (channels.find(name) != channels.end())
		return channels[name];
	return NULL;
}

Client &Server::get_client(int fd){
	std::map<int, Client *>::iterator it = clients.find(fd);
	return *it->second;
}

void Server::_ToAll(int ori_fd, std::string message){
	std::set<std::string> channelList = findInChannel(ori_fd);
	while (!channelList.empty()){
		std::string channelName = *channelList.begin();
		std::map<int, Client *> all_users = channels[channelName]->get_users();
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
	std::map<int, Client *> all_users = channel->get_users();
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
		std::map<int, Client *> users = it->second->get_users();
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

// void server::set_buffer(std::string buffer)
// {
// 	this->_buffer.append(buffer);
// }
