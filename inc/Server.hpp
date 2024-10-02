#ifndef SERVER_HPP
#define SERVER_HPP

#include "colors.hpp"
#include "Channel.hpp"
#include <string>

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

	void setPass(std::string pass);
	void funct_NewClient(int i);
	void funct_NotNewClient(int i);


//* Auxiliar functions
	void _ToAll(int ori_fd, std::string message);
	void _ToAll(Channel *channel, int ori_fd, std::string message);
	int _sendall(int destfd, std::string message);
	// void broadcast_to_channel(const std::string &channelName, int sender_fd) ;
	void sendCode(int fd, std::string num, std::string nickname, std::string message);
	std::set<std::string> findInChannel(int fd);
	std::string extract_value(const std::string& line);
	void print_client(int client_fd, std::string str);
	// std::vector<std::string> parser(const std::string &commands);

public: // Handdle Commands
	void handleAuth(int fd);
	void handleOper(int fd);
	void handlePass(int fd, std::istringstream &command);
	void handleNick(int fd, std::istringstream &command);
	void handleUser(int fd, std::istringstream &command);
	void handleJoin(int fd, std::istringstream &command);
	void handlePart(int fd, std::istringstream &command);
	void handlePrivmsg(int fd, std::istringstream &command);
	void handleQuit(int fd, std::istringstream &command);
	void handleMode(int fd, std::istringstream &command);
	void handleKick(int fd, std::istringstream &command);
	void handleInvite(int fd, std::istringstream &command);
	void handleTopic(int fd, std::istringstream &command);
	
	void handleNames(int fd, std::istringstream &command);
	void handleList(int fd, std::istringstream &command);
	void handleWho(int fd, std::istringstream &command);
	void handleWhois(int fd, std::istringstream &command);
	void handleMotd(int fd, std::istringstream &command);
	void handleAway(int fd, std::istringstream &command);
	void handlePing(int fd, std::istringstream &command);
	void handlePong(int fd, std::istringstream &command);
	void handleNotice(int fd, std::istringstream &command);
	void handleTime(int fd, std::istringstream &command);
	void handleVersion(int fd, std::istringstream &command);
	void handleStats(int fd, std::istringstream &command);
	void handleLinks(int fd, std::istringstream &command);
	void handleConnect(int fd, std::istringstream &command);
	void handleTrace(int fd, std::istringstream &command);
	void handleAdmin(int fd, std::istringstream &command);
	void handleInfo(int fd, std::istringstream &command);
	void handleServlist(int fd, std::istringstream &command);
	void handleSquery(int fd, std::istringstream &command);
	void handleWhoWas(int fd, std::istringstream &command);
	void handleKill(int fd, std::istringstream &command);
	void handleError(int fd, std::istringstream &command);
	void handleRestart(int fd, std::istringstream &command);
	void handleDie(int fd, std::istringstream &command);
	void handleLusers(int fd, std::istringstream &command);
	void handleWallops(int fd, std::istringstream &command);
	void handleUserhost(int fd, std::istringstream &command);
	void handleIson(int fd, std::istringstream &command);
	void handleCap(int fd, std::istringstream &command);
	void handleSasl(int fd, std::istringstream &command);
};

#include "Client.hpp"

#endif
