#include "Bot.hpp"

Bot::Bot(int fd) : _name("Bot"), _mask(":Bot!Bot@Terracotta "), _bot_fd(fd){}


Bot::Bot(): _name("Bot"), _mask(":Bot!Bot@Terracotta "), _bot_fd(0){}

Bot::~Bot(){}


void Bot::set_addr(struct sockaddr_storage value){
	_bot_addr = value;
}


void Bot::add_channel(const std::string &channelName, Channel &channel){
	if (channels.find(channelName) == channels.end()){
		std::cout << _RED << "LEAK HERE" << _END << std::endl;
		channels.insert(std::pair<std::string, Channel *>(channelName, &channel));
	}
}

void Bot::remove_channel(const std::string &channelName){
	if (channels.find(channelName) != channels.end()){
		channels.erase(channelName);
	}
}


int const &Bot::get_bot_fd() const{ return _bot_fd; }

