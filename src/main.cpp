#include "colors.hpp"

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

	//int _max = 0;
	int socketServer, newsocket;
	int port = 6667;
	std::string pass = "banana123";
	struct sockaddr_in addr, client;
	char buffer[1024];

	struct pollfd fds[128];
	int nfds = 1;

	for (int i = 0; i < 128; i++) {
        fds[i].fd = -1; // -1 indicates available entry
        fds[i].events = POLLIN;
    }

	/* if (argc != 3){
		std::cout << "Input: ./ircserv <port> <password>" << std::endl;
		return 1;
	} */
	(void)argv;
	(void)argc;
	socketServer = socket(AF_INET,SOCK_STREAM, 0);

	int opt = 1;
    setsockopt(socketServer, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY; //*htonl(2130706433);  This coverts the number to 127.0.0.1 or in another words localhost
	addr.sin_port = htons(port);
	bind(socketServer, (const struct sockaddr *)&addr, sizeof(addr));
	listen(socketServer, 10);

	fds[0].fd = socketServer;
    fds[0].events = POLLIN;

	socklen_t client_len = sizeof(client);

	while (true) {
        // Poll the sockets for activity
        int activity = poll(fds, nfds, -1);
        if (activity < 0) {
            std::cerr << "Poll error" << std::endl;
            break;
        }
		if (fds[0].revents & POLLIN) {
            newsocket = accept(socketServer, (struct sockaddr*)&client, &client_len);
            if (newsocket < 0) {
                std::cerr << "Error accepting new connection" << std::endl;
                continue;
            }
            std::cout << "New connection from "
                      << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port) << std::endl;

            // Add the new socket to the pollfd array
            for (int i = 1; i < 128; i++) {
                if (fds[i].fd == -1) {
                    fds[i].fd = newsocket;
                    fds[i].events = POLLIN;
                    nfds = std::max(nfds, i + 1); // Update nfds
                    break;
                }
            }
		}
		for (int i = 1; i < nfds; i++) {
            if (fds[i].fd != -1 && (fds[i].revents & POLLIN)) {
                int valread = recv(fds[i].fd, buffer, 1024, 0);
                if (valread <= 0) {
                    // Client disconnected, or error occurred
                    std::cout << "Client disconnected" << std::endl;
                    close(fds[i].fd);
                    fds[i].fd = -1; // Mark as available
                } else {
                    // Process the received data
                    buffer[valread] = '\0';
                    std::cout << "Received: " << buffer << std::endl;

                    // Echo the message back to the client
                    send(fds[i].fd, buffer, valread, 0);
                }
            }
		}
	}

	close(socketServer);
	close(newsocket);
	return 0;
}
