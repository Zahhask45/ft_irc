#include "Server.hpp"

void Server::handleKick(int fd, std::istringstream &command) {
	std::string channelName, user;
	command >> channelName >> user;
	int user_fd = channels[channelName]->get_by_name(user);
	if (channelName.empty() || user.empty()) {
		sendCode(fd, "461", "", "Not enough parameters");
		return ;
	}
	if (channels.find(channelName) == channels.end()) {
		sendCode(fd, "401", clients[fd]->get_nick(), channelName + " :No such nick/channel");
		return ;
	}
	if (channels[channelName]->get_operators().find(fd) == channels[channelName]->get_operators().end()) {
		sendCode(fd, "482", clients[fd]->get_nick(), channelName + " :You're not channel operator");
		return ;
	}
	if (channels[channelName]->get_users().find(fd) == channels[channelName]->get_users().end()) {
		sendCode(fd, "404", clients[fd]->get_nick(), channelName + " :Cannot send to channel");
		return ;
	}
	if (channels[channelName]->get_users().find(user_fd) == channels[channelName]->get_users().end()) {
		sendCode(fd, "441", clients[fd]->get_nick(), channelName + " :They aren't on that channel");
		return ;
	}
	print_client(fd, clients[fd]->get_mask() + "KICK " + channelName + " " + user + "\r\n");
	_ToAll(channels[channelName], fd, "KICK " + channelName + " " + user + "\r\n");
	channels[channelName]->remove_user(user);
	channels[channelName]->remove_oper(user);
}
