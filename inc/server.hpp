#ifndef SERVER_HPP
#define SERVER_HPP

#include "colors.hpp"
#include "client.hpp"
#include "Channel.hpp"

class client;
class channel;

class server
{
private:
	int _socket_Server;
	int _port;
	std::string _pass;
	struct sockaddr_in _addr;
	std::map<std::string, Channel *> channels;
	std::map<int, client > clients;
	// std::vector<std::string> user;

	//! DONT KNOW WHERE TO PUT THIS YET
	struct epoll_event _eve, _events[10];
	int _epoll_fd;
	int _nfds;
	char _buffer[1024];


public:
	server();
	server(int port, std::string pass);
	server(server &cp);
	~server();

	void binding();
	void loop(client &clients);
	void handleCommands(int user_fd, const std::string &command);
	void createChannel(const std::string &channelName, Channel &channel);
	
	void setUsers(std::string userName);

	std::string const &getUser()const;
	int const &get_socket() const;
	Channel *getChannel(const std::string name);
};



#endif
