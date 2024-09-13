#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"

//TODO: ADD STUFF FOR THE CLIENT USER

class Client
{
private:
	//* STUFF ABOUT THE USER
	int _client_fd;
	std::string _name;
	std::string _nick;
	std::string _pass;
	bool _auth;
	std::string _mask;

	int _socket;
	struct sockaddr_in _client_addr;

	Client(Client &cp);
	Client &operator=(Client &cp);
public:
	Client();
	Client(int fd);
	~Client();
	
	int const &get_socket() const;
	int const &get_client_fd() const;
	std::string const &get_name() const;
	std::string const &get_pass() const;
	std::string const &get_nick() const;
	std::string const &get_mask() const;
	bool const &get_auth() const;
	std::string get_host() const;

	void set_socket(int value);
	void set_addr(struct sockaddr_in value);
	void set_client_fd(int fd);
	void set_user_info(char buffer[]);
	void set_auth(bool value);
	void set_name(std::string name);
	void set_pass(std::string pass);
	void set_nick(std::string nick);
	void set_mask(std::string mask);

};



#endif
