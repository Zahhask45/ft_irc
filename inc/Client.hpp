#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"

//TODO: ADD STUFF FOR THE CLIENT USER

class Client
{
private:
	//* STUFF ABOUT THE USER
	int _client_fd;
	std::string _user;
	std::string _nick;
	std::string _pass;
	std::string _host;
	std::string _mask;
	std::string _realname;
	bool _auth;
	bool _isOperator;

		struct sockaddr_storage _client_addr;
		socklen_t _client_len;
		std::map<std::string, Channel *> channels;

		Client(Client &cp);
		Client &operator=(Client &cp);
	public:
		Client();
		Client(int fd);
		~Client();


	//! CHANGE THIS TO PRIVATE, REMAKE THE FUCNTIONS WHERE THIS IS USED ON
	char _buffer[1024];
	unsigned long bytes;


	public: //GETTERS
		int const &get_client_fd() const;
		std::string const &get_user() const;
		std::string const &get_pass() const;
		std::string const &get_nick() const;
		bool const &get_auth() const;
		bool const &get_is_operator() const;
		std::string const &get_host() const;
		std::string const &get_mask() const;
		std::string const &get_realname() const;
		char *get_buffer();
		int const &get_bytes() const;

	public: //SETTERS
		void setAddr(struct sockaddr_storage value);
		void setClientFd(const int &fd);
		void setAuth(bool value);
		void set_is_operator(bool value);
		void set_user(const std::string &name);
		void set_pass(const std::string &pass);
		void set_nick(const std::string &nick);
		void set_mask(const std::string &mask);
		void set_realname(const std::string &realname);
		void set_buffer(const char buffer[]);
		void set_bytes(const int &bytes);
		void set_buffer_char(size_t index, char value);

		void add_channel(const std::string &channelName, Channel &channel);
		void remove_channel(const std::string &channelName);
		void reset_bytes();
		void reset_buffer();
};

#endif
