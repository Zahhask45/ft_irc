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

/* void Server::handleAuth(int fd)
{
			// std::cout << "PASS SERVER: " << _pass << std::endl;
			// std::cout << "GET PASS: " << clients[fd]->get_pass() << std::endl;
			std::string pass = clients[fd]->get_pass();
			std::cout << strcmp(pass.c_str(), _pass.c_str()) << std::endl;
			if (strcmp(pass.c_str(), _pass.c_str()) == 0){
				sendCode(fd, "371", clients[fd]->get_nick(), ": User is Authenticated");
				clients[fd]->set_auth(true);
				return ;
			}
		} */

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
	if (this->clients[fd]->get_nick().empty()) {
		this->clients[fd]->set_nick(nick);
	}
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
	command >> username >> hostname >> servername;
	std::getline(command, realname); // Get the rest of the line
	if (username.empty()){
		sendCode(fd, "461", "", "Not enough parameters");
		return;
	}
	if (clients[fd]->get_user().empty()){
		this->clients[fd]->set_user(username);
		this->clients[fd]->set_realname(extract_value(realname));
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
	// if (channelName[0] != '#')
	// 	channelName = "#" + channelName;
	if (getChannel(channelName) == NULL)
		createChannel(channelName);
	this->clients[fd]->addChannel(channelName, *this->channels[channelName]);
	this->channels[channelName]->addUser(getClient(fd));
	print_client(fd, clients[fd]->get_mask() + "JOIN :" + channelName + "\r\n");

	sendCode(fd, "332", clients[fd]->get_nick(), channelName + " :Welcome to " + channelName);
	sendCode(fd, "353", clients[fd]->get_nick() + " = " + channelName, this->channels[channelName]->listAllUsers());
	sendCode(fd, "366", clients[fd]->get_nick(), channelName + " :End of /NAMES list");
	_ToAll(this->channels[channelName], fd, "JOIN :" + channelName + "\r\n");
}
