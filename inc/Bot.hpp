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
	struct sockaddr_storage _bot_addr;
	//socklen_t _bot_len;
	std::map<std::string, Channel *> channels;

	//* I like your funny words magic man
	//std::list<std::string> _words;
public:
	Bot();
	Bot(int fd);
	~Bot();

	void add_channel(const std::string &channelName, Channel &channel);
	void remove_channel(const std::string &channelName);


	void set_addr(struct sockaddr_storage value);

	int const &get_bot_fd() const;


};




#endif
