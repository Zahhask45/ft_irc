#include "Server.hpp"

void Server::handleAuth(int fd){
	if (clients[fd] && (clients[fd]->get_user().empty() 
		|| clients[fd]->get_pass().empty() 
		|| clients[fd]->get_nick().empty())){
		sendCode(fd, "461", "", "Not enough parameters");
		return;
	}
	if (strcmp(clients[fd]->get_pass().c_str(), _pass.c_str()) == 0){
		sendCode(fd, "001", clients[fd]->get_nick(), "Welcome to the Internet Relay Network");
		sendCode(fd, "002", clients[fd]->get_nick(), "Your host is " + clients[fd]->get_host() + ", running version 1.0");
		sendCode(fd, "003", clients[fd]->get_nick(), "This server was created " + clients[fd]->get_host());
		sendCode(fd, "004", clients[fd]->get_nick(), "BANANANA 1.0 BANANUDO ENTRAR_E_USAR " + clients[fd]->get_host());
		sendCode(fd, "005", clients[fd]->get_nick(), "This server was created " + clients[fd]->get_host());
		sendCode(fd, "371", clients[fd]->get_nick(), "User is Authenticated");
		sendCode(fd, "375", clients[fd]->get_nick(), "Message of the day - " + clients[fd]->get_host());
		sendCode(fd, "372", clients[fd]->get_nick(), "Message of the day - " + clients[fd]->get_host());
		sendCode(fd, "376", clients[fd]->get_nick(), "End of MOTD command");
		clients[fd]->set_auth(true);
		return ;
	}
}

// void Server::handleAuth(int fd)
// {
// 	// std::cout << "PASS SERVER: " << _pass << std::endl;
// 	// std::cout << "GET PASS: " << clients[fd]->get_pass() << std::endl;
// 	std::string pass = clients[fd]->get_pass();
// 	std::cout << strcmp(pass.c_str(), _pass.c_str()) << std::endl;
// 	if (strcmp(pass.c_str(), _pass.c_str()) == 0){
// 		sendCode(fd, "371", clients[fd]->get_nick(), ": User is Authenticated");
// 		clients[fd]->set_auth(true);
// 		return ;
// 	}
// }

void Server::handlePass(int fd, std::istringstream &command){
	std::string pass;
	command >> pass;
	pass = extract_value(pass);
	if (pass.empty())
		sendCode(fd, "461", "", "Not enough parameters");
	else if (clients[fd]->get_auth() == true)
		sendCode(fd, "462", clients[fd]->get_nick(), ": You may not reregister");
	else if (_pass != pass)
		sendCode(fd, "464", clients[fd]->get_nick(), ": Password incorrect");
	else {
		clients[fd]->set_pass(pass);
		sendCode(fd, "338", clients[fd]->get_nick(), ": Password accepted");
	}
}

void Server::handleNick(int fd, std::istringstream &command){
	std::string nick;
	command >> nick;
	if (nick.empty()){
		sendCode(fd, "431", "", "No nickname given");
		return;
	}
	if (this->clients[fd]->get_nick().empty())
		this->clients[fd]->set_nick(nick);
	else{
		std::string changeNick = ":" + this->clients[fd]->get_nick() + " NICK " + nick + "\r\n";
		std::string nickChangeMsg = this->clients[fd]->get_mask() + "NICK :" + nick + "\r\n";
		print_client(fd, changeNick);
		this->clients[fd]->set_nick(nick);
		_ToAll(fd, nickChangeMsg);
	}
	clients[fd]->set_mask(":" + clients[fd]->get_nick() + "!" + clients[fd]->get_user() + "@" + clients[fd]->get_host() + " ");
}

void Server::handleUser(int fd, std::istringstream &command){
	std::string username, hostname, servername, realname;
	command >> username >> hostname >> servername; // user 5 * :realname de fato 
	std::getline(command, realname); // Get the rest of the line
	if (username.empty()){
		sendCode(fd, "461", "", "Not enough parameters");
		return;
	}
	if (clients[fd]->get_user().empty()){
		this->clients[fd]->set_user(username);
		this->clients[fd]->set_realname(realname.substr(2, realname.size() - 3));
		if (clients[fd]->get_nick() != "\0")
			clients[fd]->set_mask(":" + clients[fd]->get_nick() + "!" + clients[fd]->get_user() + "@" + clients[fd]->get_host() + " ");
	}
	else{
		sendCode(fd, "462", clients[fd]->get_nick(), ": You may not reregister");
	}
}

void Server::handleJoin(int fd, std::istringstream &command){
	std::string channelName;
	command >> channelName;

	if (clients[fd]->get_auth() == false){
		print_client(fd, "Need to Auth the user\n");
		sendCode(fd, "451", clients[fd]->get_nick(), ": You have not registered");
		return ;
	}
	if (channelName.empty()){
		print_client(fd, "Channel name is empty\n");
		return ;
	}
	if (channelName[0] != '#')
		channelName = "#" + channelName;
	if (getChannel(channelName) == NULL)
		createChannel(channelName, fd);
	this->clients[fd]->addChannel(channelName, *this->channels[channelName]);
	if (this->clients[fd]->get_isOperator() == true)
				this->channels[channelName]->addOperator(getClient(fd));
	else
		this->channels[channelName]->addUser(getClient(fd));
	print_client(fd, clients[fd]->get_mask() + "JOIN :" + channelName + "\r\n");

	sendCode(fd, "332", clients[fd]->get_nick(), channelName + " :Welcome to " + channelName);
	sendCode(fd, "353", clients[fd]->get_nick() + " = " + channelName, this->channels[channelName]->listAllUsers());
	sendCode(fd, "366", clients[fd]->get_nick(), channelName + " :End of /NAMES list");
	_ToAll(this->channels[channelName], fd, "JOIN :" + channelName + "\r\n");
}

void Server::handlePrivmsg(int fd, std::istringstream &command){
	std::string target, message;
	command >> target;
	std::getline(command, message);
	if (target.empty() || message.empty()){
		sendCode(fd, "411", clients[fd]->get_nick(), ": No recipient given (PRIVMSG)");
		return ;
	}
	if (target[0] == '#'){
		if (channels.find(target) == channels.end()){
			sendCode(fd, "401", clients[fd]->get_nick(), target + " :No such nick/channel");
			return ;
		}
		if (channels[target]->getUsers().find(fd) == channels[target]->getUsers().end()){
			sendCode(fd, "404", clients[fd]->get_nick(), target + " :Cannot send to channel");
			return ;
		}
		_ToAll(channels[target], fd, "PRIVMSG " + target + " " + message + "\n");
	}
	else{
		int receiver_fd = 0;
		std::map<int, Client *>::iterator it = this->clients.begin();
		while(it != this->clients.end())
		{
			if (it->second->get_nick() == target){
					receiver_fd = it->second->get_client_fd();
				break;
			}
			it++;
		}
		// _sendall(receiver_fd, clients[fd]->get_mask() + "PRIVMSG " + target + " " + message + "\n");
		if(receiver_fd)
			print_client(receiver_fd, clients[fd]->get_mask() + "PRIVMSG " + target + " " + message + "\n");
	}
}

void Server::handlePart(int fd, std::istringstream &command){
	std::string channelName;
	command >> channelName;
	if (channelName.empty()){
		print_client(fd, "Channel name is empty\n");
		return ;
	}
	// if (channelName[0] != '#')
	// 	channelName = "#" + channelName;
	if (channels.find(channelName) == channels.end()){
		print_client(fd, "Channel does not exist\n");
		return ;
	}
	Channel *channel = channels[channelName];
	if (channel->getUsers().find(fd) == channel->getUsers().end()){
		print_client(fd, "You are not in this channel\n");
		return ;
	}
	print_client(fd, clients[fd]->get_mask() + "PART " + channelName + "\r\n");
	_ToAll(channel, fd, "PART " + channelName + "\r\n");
	channel->removeUser(clients[fd]->get_nick());
	channel->removeOper(clients[fd]->get_nick());
	clients[fd]->removeChannel(channelName);
}

void Server::handleQuit(int fd, std::istringstream &command){
	std::string message;
	command >> message;
	std::string response = clients[fd]->get_mask() + "QUIT :" + message + "\r\n";
	_ToAll(fd, response);
	for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); it++){
		if (it->second->getUsers().find(fd) != it->second->getUsers().end()){
			it->second->removeUser(clients[fd]->get_nick());
			it->second->removeOper(clients[fd]->get_nick());
		}
	}
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, clients[fd]->get_client_fd(), NULL) == -1) {
		std::cerr << "Error removing socket from epoll(quit): " << strerror(errno) << std::endl;
	}
	close(clients[fd]->get_client_fd());
	this->clients.erase(clients[fd]->get_client_fd());
	this->_cur_online--;
	this->_events[fd].data.fd = this->_events[this->_cur_online].data.fd;
	print_client(fd, response);
}

void Server::handleOper(int fd){
	if (clients[fd]->get_auth() == false){
		print_client(fd, "Need to Auth the user\n");
		sendCode(fd, "451", clients[fd]->get_nick(), ": You have not registered");
		return ;
	}
	this->clients[fd]->set_isOperator(true);
}

// TODO: Implement this function
/* void Server::handleMode(int fd, std::istringstream &command){
	std::string target, mode, arg;
	command >> target >> mode >> arg;
	int user_fd = channels[target]->getByName(arg);
	if (target.empty() || mode.empty()){
		sendCode(fd, "461", "", "Not enough parameters");
		return ;
	}
	if (target[0] == '#'){
		if (channels.find(target) == channels.end()){
			sendCode(fd, "401", clients[fd]->get_nick(), target + " :No such nick/channel");
			return ;
		}
		if (channels[target]->getUsers().find(fd) == channels[target]->getUsers().end()){
			sendCode(fd, "404", clients[fd]->get_nick(), target + " :Cannot send to channel");
			return ;
		}
		if (mode == "+o"){
			channels[target]->addOperator(*clients[fd]);
			_ToAll(channels[target], fd, "MODE " + target + " +o " + clients[fd]->get_nick() + "\r\n");
		}
		else if (mode == "-o"){
			channels[target]->removeOper(clients[fd]->get_nick());
			_ToAll(channels[target], fd, "MODE " + target + " -o " + clients[fd]->get_nick() + "\r\n");
		}
	}
	else{
		sendCode(fd, "501", clients[fd]->get_nick(), ": Unknown MODE flag");
	}
}
 */

void Server::handleKick(int fd, std::istringstream &command){
	std::string channelName, user;
	command >> channelName >> user;
	int user_fd = channels[channelName]->getByName(user);
	// Se o canal ou o usuário estiver vazio, envie um erro
	if (channelName.empty() || user.empty()){
		sendCode(fd, "461", "", "Not enough parameters");
		return ;
	}
	// Se o canal não existir, envie um erro
	if (channels.find(channelName) == channels.end()){
		sendCode(fd, "401", clients[fd]->get_nick(), channelName + " :No such nick/channel");
		return ;
	}
	if (channels[channelName]->getOperators().find(fd) == channels[channelName]->getOperators().end()){
		sendCode(fd, "482", clients[fd]->get_nick(), channelName + " :You're not channel operator");
		return ;
	}
	// Se o Operador não estiver no canal, envie um erro
	if (channels[channelName]->getUsers().find(fd) == channels[channelName]->getUsers().end()){
		sendCode(fd, "404", clients[fd]->get_nick(), channelName + " :Cannot send to channel");
		return ;
	}
	// Se o usuário não estiver no canal, envie um erro
	if (channels[channelName]->getUsers().find(user_fd) == channels[channelName]->getUsers().end()){
		sendCode(fd, "441", clients[fd]->get_nick(), channelName + " :They aren't on that channel");
		return ;
	}
	print_client(fd, clients[fd]->get_mask() + "KICK " + channelName + " " + user + "\r\n");
	_ToAll(channels[channelName], fd, "KICK " + channelName + " " + user + "\r\n");
	channels[channelName]->removeUser(user);
	channels[channelName]->removeOper(user);
}

void Server::handleInvite(int fd, std::istringstream &command){
	std::string user, channelName;
	command >> user >> channelName;
	if (user.empty() || channelName.empty()){
		sendCode(fd, "461", "", "Not enough parameters");
		return ;
	}
	int user_fd = -1;
	for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++){
		if (it->second->get_nick() == user){
			user = it->second->get_nick();
			user_fd = it->first;
			break;
		}
	}
	if (user_fd == -1 || channels.find(channelName) == channels.end()){
		sendCode(fd, "401", clients[fd]->get_nick(), user + " :No such nick/channel");
		return ;
	}
		if (channels[channelName]->getOperators().find(fd) == channels[channelName]->getOperators().end()){
		sendCode(fd, "482", clients[fd]->get_nick(), channelName + " :You're not channel operator");
		return ;
	}
	if (channels[channelName]->getUsers().find(fd) == channels[channelName]->getUsers().end()){
		sendCode(fd, "404", clients[fd]->get_nick(), channelName + " :Cannot send to channel");
		return ;
	}
	if (channels[channelName]->getUsers().find(user_fd) != channels[channelName]->getUsers().end()){
		sendCode(fd, "443", clients[fd]->get_nick(), user + " :is already on channel");
		return ;
	}
	print_client(user_fd, clients[fd]->get_mask() + "INVITE " + user + " " + channelName + "\r\n");
	print_client(fd, clients[fd]->get_mask() + "INVITE " + user + " " + channelName + "\r\n");
}

void Server::handlePing(int fd, std::istringstream &command){
	std::string server;
	command >> server;
	if (server.empty()){
		sendCode(fd, "409", "", "No origin specified");
		return ;
	}
	print_client(fd, "PONG " + server + "\r\n");
}