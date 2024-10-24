#include "Bot.hpp"

Bot::Bot(int fd) : _name("Bot"), _mask(":Bot!Bot@Terracotta "), _bot_fd(fd){}


Bot::Bot(): _name("Bot"), _user("BOT 0 * :BOT"), _pass("banana123"), _mask(":Bot!Bot@Terracotta "), _bot_fd(1){

}

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

struct sockaddr_storage const &Bot::get_addr() const{ return _bot_addr; }
std::string const &Bot::get_mask() const{ return _mask; }
std::string const &Bot::get_name() const{ return _name; }
std::string const &Bot::get_user() const{ return _user; }
std::string const &Bot::get_pass() const{ return _pass; }



void Bot::set_addr(struct sockaddr_storage value){
	_bot_addr = value;
}

void Bot::set_user(const std::string &user){
	this->_user = user;
}

void Bot::set_name(const std::string &nick){
	this->_name = nick;
}

void Bot::set_pass(const std::string &pass){
	this->_pass = pass;
}

void Bot::set_mask(const std::string &mask){
	this->_mask = mask;
}

