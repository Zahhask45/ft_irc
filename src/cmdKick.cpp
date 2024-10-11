#include "Server.hpp"

void Server::handleKick(int fd, std::istringstream &command){
	std::string channelName, user;
	command >> channelName >> user;
	int user_fd = channels[channelName]->getByName(user);
	if (channelName.empty() || user.empty()){
		sendCode(fd, "461", "", "Not enough parameters");
		return ;
	}
	if (channels.find(channelName) == channels.end()){
		sendCode(fd, "401", clients[fd]->getNick(), channelName + " :No such nick/channel");
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
	if (channels[channelName]->getUsers().find(user_fd) == channels[channelName]->getUsers().end()){
		sendCode(fd, "441", clients[fd]->getNick(), channelName + " :They aren't on that channel");
		return ;
	}
	print_client(fd, clients[fd]->getMask() + "KICK " + channelName + " " + user + "\r\n");
	_ToAll(channels[channelName], fd, "KICK " + channelName + " " + user + "\r\n");
	channels[channelName]->removeUser(user);
	channels[channelName]->removeOper(user);
}
