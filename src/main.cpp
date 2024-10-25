#include "colors.hpp"
#include "Server.hpp"

fd_set active;

void handle_signal(int signal) {
	if (SIGINT == signal || SIGQUIT == signal)
		throw std::runtime_error("Signal ^C received. Closing server.");
}

int main(int ac, char *av[]){
	if(ac == 3){	
		try{
		struct sigaction sa;
		sa.sa_flags = SA_RESTART;
		sigemptyset(&sa.sa_mask);
		sa.sa_handler = handle_signal;
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGQUIT, &sa, NULL);
	
		int port = std::atoi(av[1]);
		Server serv(port, av[2]);
	
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
