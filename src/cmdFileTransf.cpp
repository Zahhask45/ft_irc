#include "Server.hpp"
#include <arpa/inet.h>



int Server::getClientByNick(std::string nick)
{
	for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->second->get_nick() == nick)
			return it->first;
	}
	return -1;
}

void Server::handleSendFile(int fd, std::string &cmd)
{
	std::string receiver, path;
	std::istringstream command(cmd);
	command >> receiver >> path;

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		std::cerr << "Erro ao criar o socket!" << std::endl;
		return;
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(this->_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		std::cerr << "Erro ao fazer bind!" << std::endl;
		close(server_socket);
		return;
	}

	if (listen(server_socket, 1) < 0)
	{
		std::cerr << "Erro ao ouvir a porta!" << std::endl;
		close(server_socket);
		return;
	}

	std::cout << "Aguardando conexão DDC..." << std::endl;
	int client_socket = accept(server_socket, NULL, NULL);
	if (client_socket < 0)
	{
		std::cerr << "Erro ao aceitar conexão!" << std::endl;
		close(server_socket);
		return;
	}

	std::cout << "Conexão estabelecida!" << std::endl;

	std::ifstream ifs(path.c_str(), std::ios::binary);
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
			send(client_socket, buffer, bytes_read, 0);
	}

    std::cout << "Arquivo enviado com sucesso!" << std::endl;
    ifs.close();

	close(client_socket);
	close(server_socket);
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
	_file.insert(std::pair<std::string, File>(filename, file));
	print_client(fd, "File " + filename + " sent successfully.\n");
	print_client(receiver_fd, clients[fd]->get_mask() + " wants to send you a file called " + filename + ".\n");
} */


/* void server::handleAcceptFile(int fd, std::string &cmd)
{
	std::string filename, server_ip;
	std::istringstream command(cmd);
	command >> filename >> server_ip;

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		std::cerr << "Erro ao criar o socket!" << std::endl;
		return;
	}
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(this->_port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());


	
} */
	

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
