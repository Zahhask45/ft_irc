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
	if (channels[channelName]->get_operators().find(fd) == channels[channelName]->get_operators().end()){
		sendCode(fd, "482", clients[fd]->get_nick(), channelName + " :You're not channel operator");
		return ;
	}
	if (channels[channelName]->get_users().find(fd) == channels[channelName]->get_users().end()){
		sendCode(fd, "404", clients[fd]->get_nick(), channelName + " :Cannot send to channel");
		return ;
	}
	if (channels[channelName]->get_users().find(user_fd) != channels[channelName]->get_users().end()){
		sendCode(fd, "443", clients[fd]->get_nick(), user + " :is already on channel");
		return ;
	}
	if (channels[channelName]->get_invite_list().find(user_fd) != channels[channelName]->get_invite_list().end()){
		sendCode(fd, "443", clients[fd]->get_nick(), user + " :is already invited");
		return ;
	}
	channels[channelName]->add_invite(user_fd, this->clients[user_fd]);
	print_client(user_fd, clients[fd]->get_mask() + "INVITE " + user + " " + channelName + "\r\n");
	print_client(fd, clients[fd]->get_mask() + "INVITE " + user + " " + channelName + "\r\n");
}
