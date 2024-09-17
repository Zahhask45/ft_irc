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
	bool _auth;

	struct sockaddr_storage _client_addr;
	socklen_t _client_len;
	std::map<std::string, Channel *> channels;

	Client(Client &cp);
	Client &operator=(Client &cp);
public:
	Client();
	Client(int fd);
	~Client();

public: //GETTERS
	int const &get_client_fd() const;
	std::string const &get_user() const;
	std::string const &get_pass() const;
	std::string const &get_nick() const;
	bool const &get_auth() const;
	std::string const &get_host() const;
	std::string const &get_mask() const;

public: //SETTERS
	void set_addr(struct sockaddr_storage value);
	void set_client_fd(int fd);
	void set_auth(bool value);
	void set_user(std::string name);
	void set_pass(std::string pass);
	void set_nick(std::string nick);
	void set_mask(std::string mask);

	void addChannel(const std::string &channelName, Channel &channel);
	void removeChannel(const std::string &channelName);
};

#endif
