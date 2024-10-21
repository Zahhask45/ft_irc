#include "Server.hpp"

void Server::handleDCC(int fd, std::string message) {
size_t start = message.find("SEND");
        if (start == std::string::npos) {
            sendCode(fd, "400", clients[fd]->get_nick(), ":Invalid DCC command format");
            return;
        }
        
        std::string params = message.substr(start + 5);  // Skip "SEND "
        std::istringstream paramsStream(params);

        std::string filename;
        unsigned long ip, port, filesize;
        paramsStream >> filename >> ip >> port >> filesize;

        // Ensure parameters are valid
        if (filename.empty() || !ip || !port || !filesize) {
            sendCode(fd, "400", clients[fd]->get_nick(), ":Incomplete DCC parameters");
            return;
        }

        // Convert the IP address to a human-readable form
        struct in_addr ip_addr;
        ip_addr.s_addr = htonl(ip);
        char *ip_str = inet_ntoa(ip_addr);

        sendCode(fd, "NOTICE", clients[fd]->get_nick(), ":DCC SEND " + filename + " " +
                 toString(ip_str) + " " + toString(port) + " " + toString(filesize));

        // Placeholder for file transfer setup
        setupFileTransfer(filename, ip, port, filesize, fd);

        return;
}

void Server::setupFileTransfer(const std::string &filename, unsigned long ip, unsigned short port, unsigned long filesize, int fd) {
	(void)filesize;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        sendCode(fd, "400", clients[fd]->get_nick(), ":Error creating socket for file transfer");
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(ip);

    // Bind the socket to the port and IP
	int verify = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0;
    if (verify < 0) {
        sendCode(fd, "400", clients[fd]->get_nick(), ":Error binding socket for file transfer");
        close(sockfd);
        return;
    }

    // Listen for incoming connections
    if (listen(sockfd, 1) < 0) {
        sendCode(fd, "400", clients[fd]->get_nick(), ":Error setting up file transfer listener");
        close(sockfd);
        return;
    }

    sendCode(fd, "NOTICE", clients[fd]->get_nick(), ":DCC file transfer setup complete, waiting for receiver to connect.");

    // Accept the connection and start file transfer
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int new_sock = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (new_sock < 0) {
        sendCode(fd, "400", clients[fd]->get_nick(), ":Error accepting file transfer connection");
        close(sockfd);
        return;
    }

    // Open file and transfer content
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open()) {
        sendCode(fd, "400", clients[fd]->get_nick(), ":Error opening file for transfer");
        close(new_sock);
        close(sockfd);
        return;
    }

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        int sent_bytes = send(new_sock, buffer, file.gcount(), 0);
        if (sent_bytes < 0) {
            sendCode(fd, "400", clients[fd]->get_nick(), ":Error sending file data");
            break;
        }
    }

    // Clean up
    file.close();
    close(new_sock);
    close(sockfd);
}
