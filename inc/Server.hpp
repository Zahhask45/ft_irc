#ifndef SERVER_HPP
#define SERVER_HPP

#include "colors.hpp"
#include "Channel.hpp"

class Client;
class Channel;

class Server
{
private:
	int _socket_Server;
	int _port;
	std::string _pass;
	struct sockaddr_in _addr;
	std::map<std::string, Channel *> channels;
	std::map<int, Client > clients;
	// std::vector<std::string> user;

	//! DONT KNOW WHERE TO PUT THIS YET
	struct epoll_event _eve, _events[10];
	int _epoll_fd;
	int _nfds;
	char _buffer[1024];


public:
	Server();
	Server(int port, std::string pass);
	Server(Server &cp);
	~Server();

	void binding();
	void loop();
	void handleCommands(Client &client, const std::string &command);
	void createChannel(const std::string &channelName);
	
	void setUsers(std::string userName);

	std::string const &getUser()const;
	int const &get_socket() const;
	Channel *getChannel(const std::string name);
};

#include "Client.hpp"

#endif
