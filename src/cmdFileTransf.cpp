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

/* void Server::handleSendFile(int fd, std::istringstream &command)
{
	std::string receiver, path;
	command >> receiver >> path;
	if (clients[fd]->get_auth() == false){
		print_client(fd, "Need to Auth the user\n");
		sendCode(fd, "451", clients[fd]->get_nick(), ": You have not registered"); // ERR_NOTREGISTERED
		return ;
	}
	if (receiver.empty() || path.empty()){
		sendCode(fd, "461", "", "Not enough parameters"); // ERR_NEEDMOREPARAMS
		return;
	}
	int receiver_fd = getClientByNick(receiver);
	if (receiver_fd < 0){
		sendCode(fd, "401", receiver, "No such nick/channel"); // ERR_NOSUCHNICK
		return;
	}
	//verificar se o path é um arquivo
	struct stat path_stat;
    if (stat(path.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
        sendCode(fd, "999", clients[fd]->get_nick(), ":Path is a directory, not a file");
        return;
    }

	std::fstream ifs(path.c_str(), std::fstream::in);
	if (!ifs.is_open()){
		sendCode(fd, "999", clients[fd]->get_nick(), ":Invalid file path");
		return;
	}
	std::string filename = path.substr(path.find_last_of("/") + 1);
	File file(filename, clients[fd]->get_nick(), receiver, path);
	if (_file.find(filename) != _file.end()){
		sendCode(fd, "999", clients[fd]->get_nick(), ":File already exists");
		return;
	}
	_file.insert(std::make_pair(filename, file));
	print_client(fd, "File " + filename + " sent successfully.\n");
	print_client(receiver_fd, clients[fd]->get_mask() + " wants to send you a file called " + filename + ".\n");
} */

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


/* void Server::handleAcceptFile(int fd, std::istringstream &command)
{
	std::string filename, path;
	command >> filename >> path;
	if (clients[fd]->get_auth() == false){
		print_client(fd, "Need to Auth the user\n");
		sendCode(fd, "451", clients[fd]->get_nick(), ": You have not registered"); // ERR_NOTREGISTERED
		return ;
	}
	if (filename.empty() || path.empty()){
		sendCode(fd, "461", "", "Not enough parameters"); // ERR_NEEDMOREPARAMS
		return;
	}
	if (_file.find(filename) == _file.end()){
		sendCode(fd, "999", clients[fd]->get_nick(), ":File not found");
		return;
	}
	File file(_file.find(filename)->second);
	if (file.getReceiver() != clients[fd]->get_nick()){
		sendCode(fd, "999", clients[fd]->get_nick(), ":You are not the receiver of this file");
		return;
	}
	std::string pathDest = path + '/' + filename;
	std::fstream ofs(pathDest.c_str(), std::fstream::out);
	std::fstream ifs(file.getPath().c_str(), std::fstream::in);
	if (!ofs.is_open() || !ifs.is_open()){
		sendCode(fd, "999", clients[fd]->get_nick(), ":Invalid file path");
		return;
	}
	ofs << ifs.rdbuf();
	_file.erase(filename);
	print_client(fd, "File " + filename + " received successfully.\n");
}
 */
