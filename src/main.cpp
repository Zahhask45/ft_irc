#include "colors.hpp"

fd_set active;

int main(int argc, char **argv){

	int _max = 0;
	int socketServer;
	int port = 666;
	std::string pass = "banana123";
	struct sockaddr_in addr;

	if (argc != 3){
		std::cout << "Input: ./ircserv <port> <password>" << std::endl;
		return 1;
	}
	(void)argv;
	socketServer = socket(AF_INET,SOCK_STREAM, 0);
	_max = socketServer;
	FD_SET(socketServer, &active);
	return 0;
}