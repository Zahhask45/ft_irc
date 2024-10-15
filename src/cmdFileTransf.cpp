#include "Server.hpp"

int Server::getClientByNick(std::string nick)
{
	for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->second->get_nick() == nick)
			return it->first;
	}
	return -1;
}

void Server::handleSendFile(int fd, std::istringstream &command)
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
	_file.insert(std::pair<std::string, File>(filename, file));
	print_client(receiver_fd, clients[fd]->get_mask() + " wants to send you a file called " + filename + ".\n");
}

void Server::handleAcceptFile(int fd, std::istringstream &command)
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
}
