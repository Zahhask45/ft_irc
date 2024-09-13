#include "colors.hpp"
#include "Server.hpp"

fd_set active;

void handle_signal(int signal) {
	std::cout << "Signal " << signal << " received. Closing server." << std::endl;
	exit(0);
}

int main(int argc, char **argv){
	struct sigaction sa;
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa, NULL);

	Server serv(6667, "banana123");
	Client clients;

	serv.binding();
	serv.loop();
	std::string command;
	command = "NICK";
	(void)argv;
	(void)argc;
	
	close(serv.get_socket());
	close(clients.get_socket());
	return 0;
}
