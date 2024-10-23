#ifndef BOT_HPP
#define BOT_HPP

#include "colors.hpp"
#include "Server.hpp"

class Bot
{
private:
	std::string _name;
	std::string _user;
	std::string _pass;
	std::string _mask;
	int _bot_fd;
	
	std::map<std::string, Channel *> bot_channels;

	struct sockaddr_storage _bot_addr;

	//* I like your funny words magic man
	//std::list<std::string> _words;
public:
	Bot();
	Bot(int fd);
	~Bot();

	void add_channel(const std::string &channelName, Channel &channel);
	void remove_channel(const std::string &channelName);

	void set_addr(struct sockaddr_storage value);
	void set_user(const std::string &name);
	void set_pass(const std::string &pass);
	void set_name(const std::string &nick);
	void set_mask(const std::string &nick);

	struct sockaddr_storage const &get_addr() const;
	int const &get_bot_fd() const;
	std::string const &get_mask() const;
	std::string const &get_name() const;
	std::string const &get_user() const;
	std::string const &get_pass() const;



};




#endif
