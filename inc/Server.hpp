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
	struct sockaddr_storage _addr;
	std::map<std::string, Channel *> channels;
	std::map<int, Client *> clients;
	// std::vector<std::string> user;

	//! DONT KNOW WHERE TO PUT THIS YET
	struct epoll_event _eve, _events[10];
	int _epoll_fd;
	int _nfds;
	int _cur_online;
	char _buffer[1024];

	Server(Server &cp);
	Server &operator=(Server &cp);
public:
	Server();
	Server(int port, std::string pass);
	~Server();
//* Execution functions
	void binding();
	void loop();
	void handleCommands(int fd, const std::string &command);
	void createChannel(const std::string &channelName, int fd);
	
	std::string const &getUser()const;
	int const &get_socket() const;
	Channel *getChannel(const std::string name);
	Client &getClient(int fd);
	void funct_NewClient(int i);
	void funct_NotNewClient(int i);

//* Auxiliar functions
	void _ToAll(int ori_fd, std::string message);
	void _ToAll(Channel *channel, int ori_fd, std::string message);
	// void broadcast_to_channel(const std::string &channelName, int sender_fd) ;
	void sendCode(int fd, std::string num, std::string nickname, std::string message);
	std::set<std::string> findInChannel(int fd);
	std::string extract_value(const std::string& line, const std::string& key);
	void print_client(int client_fd, std::string str);
};

#include "Client.hpp"

#endif
