#ifndef BOT_HPP
#define BOT_HPP

#include "colors.hpp"
#include "Server.hpp"


class Bot
{
private:
	std::string _name;
	std::string _mask;
	int _bot_fd;
	
	std::map<std::string, Channel *> bot_channels;

	//* I like your funny words magic man
	//std::list<std::string> _words;
public:
	Bot();
	Bot(int fd);
	~Bot();

	void add_channel(const std::string &channelName, Channel &channel);
	void remove_channel(const std::string &channelName);


	int const &get_bot_fd() const;
	std::string const &get_mask() const;
	std::string const &get_name() const;



};




#endif
