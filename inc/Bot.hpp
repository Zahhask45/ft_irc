#ifndef BOT_HPP
#define BOT_HPP

#include "colors.hpp"
#include "Server.hpp"

class Bot {
	private:
		std::string _name;
		std::string _mask;
		int _bot_fd;
		
		std::map<std::string, Channel *> bot_channels;
		std::list<std::string> _welcome_messages;
	public:
		Bot();
		Bot(int fd);
		~Bot();

		void add_channel(const std::string &channelName, Channel &channel);
		void remove_channel(const std::string &channelName);

		void set_name(const std::string &nick);
		void set_mask(const std::string &nick);

		int const &get_bot_fd() const;
		std::string const &get_mask() const;
		std::string const &get_name() const;
		std::string get_random_message();

};

#endif
