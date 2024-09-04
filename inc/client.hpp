#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "server.hpp"

//TODO: ADD STUFF FOR THE CLIENT USER

class server;

class client
{
private:
	//* STUFF ABOUT THE USER
	int _client_fd;
	std::string _name;
	std::string _nick;

	int _socket;
	struct sockaddr_in _client_addr;
public:
	client();
	client(client &cp);
	~client();
	void set_socket(int value);
	int const &get_socket() const;
	void set_addr(struct sockaddr_in value);
};



#endif
