#include "colors.hpp"
#include "Server.hpp"

fd_set active;

void handle_signal(int signal) {
	if (SIGINT == signal || SIGQUIT == signal)
		throw std::runtime_error("Signal ^C received. Closing server.");
}

int main(){
	try{
	struct sigaction sa;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handle_signal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL); // Ensure SIGQUIT is also handled


	Server serv(6667, "banana123");

	serv.binding();
	serv.loop();
	
	close(serv.get_socket());
	return 0;
	}
	catch (std::runtime_error &e) {
		std::cerr << e.what() << std::endl;
	}
	return 1;
}
