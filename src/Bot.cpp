#include "Bot.hpp"

Bot::Bot(int fd) : _name("Pie"), _mask(":Pie!Pie@Terracotta "), _bot_fd(fd) {
	std::string welcome[12] = {
	"Welcome to the channel! Make yourself at home!",
	"Hey! Glad to have you here!",
	"Hello ! Nice to see you!",
	"Welcome! We hope you enjoy your time here.",
	"Look who just joined! 👋",
	"Hello! Feel free to ask questions and join the chat!",
	"Hey! Welcome aboard!",
	"It's a pleasure to have you here!",
	"Hello! Grab a seat and enjoy the chat!",
	"Welcome! We are happy you are here!",
	"Great to see you! Welcome to the group!",
	"Howdy! Jump right in and say hello!"
	};
	for (size_t i = 0; i < 12; i++) {
		_welcome_messages.push_back(welcome[i]);
	}

}


Bot::Bot(): _name("Pie"), _mask(":Pie!Pie@Terracotta "), _bot_fd(1) {
	std::string welcome[12] = {
	"Welcome to the channel! Make yourself at home!",
	"Hey! Glad to have you here!",
	"Hello ! Nice to see you!",
	"Welcome! We hope you enjoy your time here.",
	"Look who just joined! 👋",
	"Hello! Feel free to ask questions and join the chat!",
	"Hey! Welcome aboard!",
	"It's a pleasure to have you here!",
	"Hello! Grab a seat and enjoy the chat!",
	"Welcome! We are happy you are here!",
	"Great to see you! Welcome to the group!",
	"Howdy! Jump right in and say hello!"
	};
	for (size_t i = 0; i < 12; i++) {
		_welcome_messages.push_back(welcome[i]);
	}
}

Bot::~Bot() {}

void Bot::add_channel(const std::string &channelName, Channel &channel) {
	if (bot_channels.find(channelName) == bot_channels.end())
		bot_channels.insert(std::pair<std::string, Channel *>(channelName, &channel));
}

void Bot::remove_channel(const std::string &channelName) {
	if (bot_channels.find(channelName) != bot_channels.end()) {
		bot_channels.erase(channelName);
	}
}


int const &Bot::get_bot_fd() const { return _bot_fd; }

std::string const &Bot::get_mask() const { return _mask; }
std::string const &Bot::get_name() const { return _name; }

void Bot::set_name(const std::string &nick) {
	this->_name = nick;
}

void Bot::set_mask(const std::string &mask) {
	this->_mask = mask;
}

std::string Bot::get_random_message() {
	std::srand(static_cast<unsigned>(std::time(0)));
	int randomIndex = std::rand() % _welcome_messages.size();

	std::list<std::string>::const_iterator it = _welcome_messages.begin();
	std::advance(it, randomIndex);
	std::string message = *it;
	return message;
}

