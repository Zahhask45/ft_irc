#include "Server.hpp"

void Server::handleJoin(int fd, std::istringstream &command){
	std::string channelName, key, line;
	command >> line >> key;
	std::stringstream ss(line);

	if (clients[fd]->get_auth() == false){
		print_client(fd, "Need to Auth the user\n");
		sendCode(fd, "451", clients[fd]->get_nick(), ": You have not registered"); // ERR_NOTREGISTERED
		return ;
	}
	if (line.empty()){
		sendCode(fd, "461", "", "Not enough parameters"); // ERR_NEEDMOREPARAMS
		return ;
	}

	while (std::getline(ss, channelName, ',')){
		if (channelName[0] != '#' || channelName == "#"){
			sendCode(fd, "476", clients[fd]->get_nick(), channelName + " :Invalid channel name"); // ERR_BADCHANMASK
			return ;
		}
		if (get_channel(channelName) == NULL)
			create_channel(channelName, fd);
		if ((this->channels[channelName]->get_invite_channel() == true 
		&& this->channels[channelName]->get_invite_list().find(fd) == this->channels[channelName]->get_invite_list().end())
		&& this->channels[channelName]->get_operators().find(fd) == this->channels[channelName]->get_operators().end()){
			sendCode(fd, "473", clients[fd]->get_nick(), channelName + " :Cannot join channel (+i) invite only"); // ERR_INVITEONLYCHAN
			return ;
		}
		if (!this->channels[channelName]->get_key().empty() && this->channels[channelName]->get_key() != key){
			sendCode(fd, "475", clients[fd]->get_nick(), channelName + " :Cannot join channel (+k) bad key"); // ERR_BADCHANNELKEY
			return ;
		}
		if (this->channels[channelName]->get_users().size() >= this->channels[channelName]->get_limit() ){
			sendCode(fd, "471", clients[fd]->get_nick(), channelName + " :Cannot join channel (+l) limit reached"); // ERR_CHANNELISFULL
			return ;
		}



		this->clients[fd]->add_channel(channelName, *this->channels[channelName]);
		
		this->channels[channelName]->add_user(get_client(fd));
		
		print_client(fd, clients[fd]->get_mask() + "JOIN :" + channelName + "\r\n");

		sendCode(fd, "332", clients[fd]->get_nick(), channelName + " " + this->channels[channelName]->get_topic());
		sendCode(fd, "353", clients[fd]->get_nick() + " = " + channelName, this->channels[channelName]->list_all_users().append("+" + bot->get_name() + " "));
		sendCode(fd, "366", clients[fd]->get_nick(), channelName + " :End of /NAMES list");
		_ToAll(this->channels[channelName], fd, "JOIN :" + channelName + "\r\n");

		std::string welcomeMsg = "Hello, I am " + bot->get_name() + "! Welcome to " + channelName;
		std::string fullMessage = bot->get_mask() + " PRIVMSG " + channels[channelName]->get_name() + " :" + welcomeMsg + "\r\n";
		print_client(clients[fd]->get_client_fd(), fullMessage);
	}
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
		if (channels[target]->get_users().find(fd) == channels[target]->get_users().end()){
			sendCode(fd, "404", clients[fd]->get_nick(), target + " :Cannot send to channel");
			return ;
		}
		if (message.find("SHA-256") != std::string::npos){
			_ToAll(channels[target], fd, "PRIVMSG " + target + message + "\n");
		}
		else 
			_ToAll(channels[target], fd, "PRIVMSG " + target + " " + message + "\n");
	}
	else{
		std::map<int, Client *>::iterator it = this->clients.begin();
		if (bot->get_name() == target){
			print_client(bot->get_bot_fd(), clients[fd]->get_mask() + "PRIVMSG " + target + " " + message + "\n");
			if (message.find("Terracotta") || message.find("Banana") || message.find("Terracota pie")){
				std::cout << _CYAN << "DAMN I WAS HERE FAST" << _END << std::endl;
				print_client(clients[fd]->get_client_fd(), bot->get_mask() + "PRIVMSG " + clients[fd]->get_nick() + " I like you funny words magic man\r\n");
			}

			return ;
		}

		int receiver_fd = 0;
		while(it != this->clients.end())
		{
			if (it->second->get_nick() == target){
					receiver_fd = it->second->get_client_fd();
				break;
			}
			it++;
		}
		if (receiver_fd == 0) {
			sendCode(fd, "401", clients[fd]->get_nick(), target + " :No such nick/channel"); // ERR_NOSUCHNICK
			return;
        }
		if (message.find(toString("\x01") + "DCC SEND") != std::string::npos || message.find("SHA") != std::string::npos){
			// aqui o sendall tinha um " " entre target e message isso dava erro
			_sendall(receiver_fd, clients[fd]->get_mask() + "PRIVMSG " + target + message + "\n"); 
		}
		else if(receiver_fd)
			print_client(receiver_fd, clients[fd]->get_mask() + "PRIVMSG " + target + message + "\n");
	}
}

void Server::handlePart(int fd, std::istringstream &command){
	std::string channelName;
	command >> channelName;
	if (channelName.empty()){
		print_client(fd, "Channel name is empty\n");
		return ;
	}
	if (channels.find(channelName) == channels.end()){
		print_client(fd, "Channel does not exist\n");
		return ;
	}
	Channel *channel = channels[channelName];
	if (channel->get_users().find(fd) == channel->get_users().end()){
		print_client(fd, "You are not in this channel\n");
		return ;
	}
	print_client(fd, clients[fd]->get_mask() + "PART " + channelName + "\r\n");
	_ToAll(channel, fd, "PART " + channelName + "\r\n");
	channel->remove_user(clients[fd]->get_nick());
	channel->remove_oper(clients[fd]->get_nick());
	if (this->channels[channelName]->list_all_users() == ":"){
		bot->remove_channel(channelName);
		delete this->channels[channelName];
		this->channels.erase(channelName);
	}
	clients[fd]->remove_channel(channelName);
}

void Server::handleQuit(int fd, std::istringstream &command){
	std::string message;
	getline(command, message);
	std::string response = clients[fd]->get_mask() + "QUIT :" + message + "\r\n";
	_ToAll(fd, response);
	std::list<std::string> empty_channels;
	for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); it++){
		if (it->second->get_users().find(fd) != it->second->get_users().end()){
			it->second->remove_user(clients[fd]->get_nick());
			it->second->remove_oper(clients[fd]->get_nick());
			clients[fd]->remove_channel(it->first);
			if (this->channels[it->first]->list_all_users() == ":"){
				empty_channels.push_back(it->first);
			}
		}
	}
	for (std::list<std::string>::iterator it = empty_channels.begin(); it != empty_channels.end(); it++){
		if (this->channels[*it]->list_all_users() == ":"){
			bot->remove_channel(*it);
			delete this->channels[*it];
			this->channels.erase(*it);
		}
	}
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, clients[fd]->get_client_fd(), NULL) == -1) {
		std::cerr << "Error removing socket from epoll(quit): " << strerror(errno) << std::endl;
	}
	close(clients[fd]->get_client_fd());
	end_connection(fd);
	// this->clients.erase(fd); //! Sempre que um cliente sai, tem que fazer o delete dele
	// this->_cur_online--;
	this->_events[fd].data.fd = this->_events[this->_cur_online].data.fd;
	print_client(fd, response);
}

void Server::handlePing(int fd, std::istringstream &command){
	std::string server;
	command >> server;
	if (server.empty()){
		sendCode(fd, "409", "", "No origin specified");
		return ;
	}
	sendCode(fd, "PONG", clients[fd]->get_host(), server);
}

void Server::handleWho(int fd, std::istringstream &command){
	std::string mask;
	command >> mask;
	if (mask.empty()){
		sendCode(fd, "461", "", "Not enough parameters");
		return ;
	}
	std::map<int, Client *>::iterator it = clients.begin();
	while (it != clients.end()){
		if (it->second->get_mask().find(mask) != std::string::npos){
			sendCode(fd, "352", clients[fd]->get_nick(), mask + " " + it->second->get_user() + " " + it->second->get_host() + " " + it->second->get_host() + " " + it->second->get_nick() + " H :0 " + it->second->get_realname());
		}
		it++;
	}
	sendCode(fd, "315", clients[fd]->get_nick(), mask + " :End of /WHO list");
}

void Server::handleWhois(int fd, std::istringstream &command){
	std::string mask;
	command >> mask;
	if (mask.empty()){
		sendCode(fd, "461", "", "Not enough parameters");
		return ;
	}
	std::map<int, Client *>::iterator it = clients.begin();
	while (it != clients.end()){
		if (it->second->get_mask().find(mask) != std::string::npos){
			sendCode(fd, "311", clients[fd]->get_nick(), mask + " " + it->second->get_user() + " " + it->second->get_host() + " * :" + it->second->get_realname());
			sendCode(fd, "312", clients[fd]->get_nick(), mask + " " + it->second->get_host() + " :42Porto");
			sendCode(fd, "319", clients[fd]->get_nick(), mask + " :");
		}
		it++;
	}
	sendCode(fd, "318", clients[fd]->get_nick(), mask + " :End of /WHOIS list");
}

void Server::handleList(int fd){
	for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); it++){
		sendCode(fd, "322", clients[fd]->get_nick(), it->first + " " + toString(it->second->get_users().size()) + " :" + it->second->get_topic());
	}
	sendCode(fd, "323", clients[fd]->get_nick(), ":End of /LIST");
}
