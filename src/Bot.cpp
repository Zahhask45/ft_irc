#include "Bot.hpp"

Bot::Bot(int fd) : _name("Bot"), _mask(":Bot!Bot@Terracotta "), _bot_fd(fd){}


Bot::Bot(): _name("Bot"), _mask(":Bot!Bot@Terracotta "), _bot_fd(0){}

Bot::~Bot(){}



void Bot::add_channel(const std::string &channelName, Channel &channel){
	if (bot_channels.find(channelName) == bot_channels.end())
        bot_channels.insert(std::pair<std::string, Channel *>(channelName, &channel));
}

void Bot::remove_channel(const std::string &channelName){
	if (bot_channels.find(channelName) != bot_channels.end()){
		bot_channels.erase(channelName);
	}
}


int const &Bot::get_bot_fd() const{ return _bot_fd; }

std::string const &Bot::get_mask() const{ return _mask; }
std::string const &Bot::get_name() const{ return _name; }


