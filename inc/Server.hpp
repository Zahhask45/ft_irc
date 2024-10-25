#ifndef SERVER_HPP
#define SERVER_HPP

#include "colors.hpp"
#include "Channel.hpp"
#include "Bot.hpp"
#include <fstream>

#define MAX_CLIENTS 100

class Client;
class Channel;
class Bot;

class Server
{
	private:
		int _socket_Server;
		int _port;
		std::string _pass;
		std::map<std::string, Channel *> channels;
		std::map<int, Client *> clients;
		Bot* bot;

		struct epoll_event _eve, _events[MAX_CLIENTS];
		int _epoll_fd;
		int _nfds;
		int _cur_online;

		Server(Server &cp);
		Server &operator=(Server &cp);
	public:
		Server();
		Server(int port, std::string pass);
		~Server();
	//* Execution functions
		void binding();
		void loop();
		void handle_commands(int fd, const std::string &command);
		void create_channel(const std::string &channelName, int fd);
		
		std::string const &get_user()const;
		int const &get_socket() const;
		Channel *get_channel(const std::string name);
		Client &get_client(int fd);
		Bot &get_bot();

		void set_pass(std::string pass);
		void funct_bot();
		void funct_new_client(int i);
		void funct_not_new_client(int i);
		int	checkAuth(int fd);


	//* Auxiliar functions
		void _ToAll(int ori_fd, std::string message);
		void _ToAll(Channel *channel, int ori_fd, std::string message);
		int _sendall(int destfd, std::string message);
		void sendCode(int fd, std::string num, std::string nickname, std::string message);
		std::set<std::string> findInChannel(int fd);
		void print_client(int client_fd, std::string str);
		void end_connection(int fd);

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
		void handleList(int fd);
		void handleWho(int fd, std::istringstream &command);
		void handleWhois(int fd, std::istringstream &command);
		
		void handlePing(int fd, std::istringstream &command);

		//checkMode
		int checkMode(int fd, std::string &target, std::string &mode, std::string &arg);
		void genericSendMode(int fd, std::string target, char mode, std::string arg, char sign);
		bool findNick(std::string nick);
		int getClientByNick(std::string nick);
};

#include "Client.hpp"
#include "Utils.tpp"

#endif
