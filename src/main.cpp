#include "colors.hpp"
#include "Server.hpp"

fd_set active;

void handle_signal(int signal) {
	if (SIGINT == signal || SIGQUIT == signal)
		throw std::runtime_error("Signal ^C received. Closing server.");
}

int parsing_port(char *port){
	int i = 0;
	while (port[i]){
		if (!isdigit(port[i]))
			return -1;
		i++;
	}
	return atoi(port);
}

int main(int argc, char **argv){
	if(argc == 3){	
		try{
		struct sigaction sa;
		sa.sa_flags = SA_RESTART;
		sigemptyset(&sa.sa_mask);
		sa.sa_handler = handle_signal;
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGQUIT, &sa, NULL);
		int port = parsing_port(argv[1]);
		if (port == -1)
			throw std::runtime_error("Invalid port number.");
		Server serv(port, argv[2]);
	
		serv.binding();
		serv.loop();
		
		close(serv.get_socket());
		}
		catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
			return 0;
		}
	}
	else
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
	return 1;
}
