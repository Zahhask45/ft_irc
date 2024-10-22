#include "Server.hpp"
// #include <arpa/inet.h>

int Server::getClientByNick(std::string nick)
{
	for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->second->get_nick() == nick)
			return it->first;
	}
	return -1;
}

void Server::handleSendFile(int fd, const std::string &cmd, const std::string &target)
{
	std::string sha, check, junk3, filename;
	std::istringstream command(cmd);
	command >> sha >> sha >> junk3 >> filename ;

	int receiver_fd = getClientByNick(target);
	if (receiver_fd < 0){
		sendCode(fd, "401", target, "No such nick/channel"); // ERR_NOSUCHNICK
		return;
	}

	std::ifstream ifs(filename.c_str(), std::ios::binary);
	if (!ifs.is_open()){
		sendCode(fd, "999", clients[fd]->get_nick(), ":Invalid file path");
		return;
	}

	char buffer[1024];
	while (!ifs.eof())
	{
		ifs.read(buffer, 1024);
		int bytes_read = ifs.gcount();
		if (bytes_read > 0)
		{
			::send(receiver_fd, buffer, bytes_read, 0);
		}
	}

    std::cout << "Arquivo enviado com sucesso!" << std::endl;
    ifs.close();
}


void Server::handleAcceptFile(int fd, std::string &cmd, const std::string &target)
{
	std::string dcc, send, filename;
	std::istringstream command(cmd);
	command >> dcc >> send >> filename;

	//" :\001DCC SEND \"ex00  em.cpp\" 168558851 40015 1019\001\r"

	std::string reply;
	reply = "NOTICE " + target + " :" + toString("\x01") + \
		"DCC GET " + clients[fd]->get_nick() + " " + filename + toString("\x01");

	size_t total = 0;
	while (total != reply.length()){
		ssize_t nb = ::send(fd, reply.c_str() + total, reply.length() - total, 0);
		if (nb == -1)
			std::cout << "send error" << std::endl;
		total += nb;
	}	
}

