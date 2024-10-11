#include "Server.hpp"

void Server::handleInvite(int fd, std::istringstream &command){
	std::string user, channelName;
	command >> user >> channelName;
	if (user.empty() || channelName.empty()){
		sendCode(fd, "461", "", "Not enough parameters");
		return ;
	}
	int user_fd = -1;
	for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++){
		if (it->second->getNick() == user){
			user = it->second->getNick();
			user_fd = it->first;
			break;
		}
	}
	if (user_fd == -1 || channels.find(channelName) == channels.end()){
		sendCode(fd, "401", clients[fd]->getNick(), user + " :No such nick/channel");
		return ;
	}
	if (channels[channelName]->getOperators().find(fd) == channels[channelName]->getOperators().end()){
		sendCode(fd, "482", clients[fd]->getNick(), channelName + " :You're not channel operator");
		return ;
	}
	if (channels[channelName]->getUsers().find(fd) == channels[channelName]->getUsers().end()){
		sendCode(fd, "404", clients[fd]->getNick(), channelName + " :Cannot send to channel");
		return ;
	}
	if (channels[channelName]->getUsers().find(user_fd) != channels[channelName]->getUsers().end()){
		sendCode(fd, "443", clients[fd]->getNick(), user + " :is already on channel");
		return ;
	}
	if (channels[channelName]->getInviteList().find(user_fd) != channels[channelName]->getInviteList().end()){
		sendCode(fd, "443", clients[fd]->getNick(), user + " :is already invited");
		return ;
	}
	channels[channelName]->addInvite(user_fd, this->clients[user_fd]);
	print_client(user_fd, clients[fd]->getMask() + "INVITE " + user + " " + channelName + "\r\n");
	print_client(fd, clients[fd]->getMask() + "INVITE " + user + " " + channelName + "\r\n");
}
