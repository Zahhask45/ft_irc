#include "Server.hpp"

void Server::handleTopic(int fd, std::istringstream &command){
	std::string channelName, topic;
	command >> channelName;
	std::getline(command, topic);
	if (channelName.empty()){
		sendCode(fd, "461", "", "Not enough parameters");
		return ;
	}
	if (channels.find(channelName) == channels.end()){
		sendCode(fd, "401", clients[fd]->get_nick(), channelName + " :No such nick/channel");
		return ;
	}
	if (channels[channelName]->get_operators().find(fd) == channels[channelName]->get_operators().end() && channels[channelName]->get_modes().find('t') != std::string::npos){
		sendCode(fd, "482", clients[fd]->get_nick(), channelName + " :You're not channel operator");
		return ;
	}
	if (channels[channelName]->get_users().find(fd) == channels[channelName]->get_users().end()){
		sendCode(fd, "442", clients[fd]->get_nick(), channelName + " :You're not on that channel");
		return ;
	}
	if (topic.empty()){
		sendCode(fd, "331", clients[fd]->get_nick(), channelName + " :No topic is set");
		return ;
	}
	channels[channelName]->set_topic(topic);
	sendCode(fd, "332", clients[fd]->get_nick(), channelName + topic);
	_ToAll(channels[channelName], fd, "TOPIC " + channelName + topic + "\r\n");
}
