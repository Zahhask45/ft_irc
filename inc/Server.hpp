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
	void createChannel(const std::string &channelName);
	
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

public: // Handdle Commands
	void handleAuth(int fd);
	void handlePass(int fd, std::string command);
	void handleNick(int fd, std::string command);
	void handleUser(int fd, std::string command);
	void handleJoin(int fd, std::string command);
	void handlePart(int fd, std::string command);
	void handlePrivmsg(int fd, std::string command);
	void handleQuit(int fd, std::string command);
	void handleNames(int fd, std::string command);
	void handleList(int fd, std::string command);
	void handleWho(int fd, std::string command);
	void handleWhois(int fd, std::string command);
	void handleMode(int fd, std::string command);
	void handleKick(int fd, std::string command);
	void handleTopic(int fd, std::string command);
	void handleMotd(int fd, std::string command);
	void handleAway(int fd, std::string command);
	void handlePing(int fd, std::string command);
	void handlePong(int fd, std::string command);
	void handleNotice(int fd, std::string command);
	void handleInvite(int fd, std::string command);
	void handleTime(int fd, std::string command);
	void handleVersion(int fd, std::string command);
	void handleStats(int fd, std::string command);
	void handleLinks(int fd, std::string command);
	void handleConnect(int fd, std::string command);
	void handleTrace(int fd, std::string command);
	void handleAdmin(int fd, std::string command);
	void handleInfo(int fd, std::string command);
	void handleServlist(int fd, std::string command);
	void handleSquery(int fd, std::string command);
	void handleWhoWas(int fd, std::string command);
	void handleKill(int fd, std::string command);
	void handleError(int fd, std::string command);
	void handleRestart(int fd, std::string command);
	void handleDie(int fd, std::string command);
	void handleOper(int fd, std::string command);
	void handleLusers(int fd, std::string command);
	void handleWallops(int fd, std::string command);
	void handleUserhost(int fd, std::string command);
	void handleIson(int fd, std::string command);
	void handleCap(int fd, std::string command);
	void handleSasl(int fd, std::string command);
};

#include "Client.hpp"

#endif
