#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "server.hpp"

//TODO: ADD STUFF FOR THE CLIENT USER

class server;

class Client
{
private:
	//* STUFF ABOUT THE USER
	int _client_fd;
	std::string _name;
	std::string _nick;
	std::string _pass;
	bool _auth;

	int _socket;
	struct sockaddr_in _client_addr;

public:
	Client();
	Client(Client &cp);
	~Client();
	
	int const &get_socket() const;
	int const &get_client_fd() const;
	std::string const &get_name() const;
	std::string const &get_pass() const;
	std::string const &get_nick() const;
	bool const &get_auth() const;

	void set_socket(int value);
	void set_addr(struct sockaddr_in value);
	void set_client_fd(int fd);
	void set_user_info(char buffer[]);
	void set_auth(bool value);
};



#endif
