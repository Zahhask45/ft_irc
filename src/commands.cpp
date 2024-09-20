#include "Server.hpp"

void Server::handleAuth(int fd){
	if (clients[fd] && (clients[fd]->get_user().empty() 
		|| clients[fd]->get_pass().empty() 
		|| clients[fd]->get_nick().empty())){
		sendCode(fd, "461", "", "Not enough parameters");
		return;
	}
	if (strcmp(clients[fd]->get_pass().c_str(), _pass.c_str()) == 0){
		sendCode(fd, "001", clients[fd]->get_nick(), "Welcome to the Internet Relay Network");
		sendCode(fd, "002", clients[fd]->get_nick(), "Your host is " + clients[fd]->get_host() + ", running version 1.0");
		sendCode(fd, "003", clients[fd]->get_nick(), "This server was created " + clients[fd]->get_host());
		sendCode(fd, "004", clients[fd]->get_nick(), "BANANANA 1.0 BANANUDO ENTRAR_E_USAR " + clients[fd]->get_host());
		sendCode(fd, "005", clients[fd]->get_nick(), "This server was created " + clients[fd]->get_host());
		sendCode(fd, "371", clients[fd]->get_nick(), "User is Authenticated");
		sendCode(fd, "375", clients[fd]->get_nick(), "Message of the day - " + clients[fd]->get_host());
		sendCode(fd, "372", clients[fd]->get_nick(), "Message of the day - " + clients[fd]->get_host());
		sendCode(fd, "376", clients[fd]->get_nick(), "End of MOTD command");
		clients[fd]->set_auth(true);
		return ;
	}
}

/* void Server::handleAuth(int fd)
{
			// std::cout << "PASS SERVER: " << _pass << std::endl;
			// std::cout << "GET PASS: " << clients[fd]->get_pass() << std::endl;
			std::string pass = clients[fd]->get_pass();
			std::cout << strcmp(pass.c_str(), _pass.c_str()) << std::endl;
			if (strcmp(pass.c_str(), _pass.c_str()) == 0){
				sendCode(fd, "371", clients[fd]->get_nick(), ": User is Authenticated");
				clients[fd]->set_auth(true);
				return ;
			}
		} */

void Server::handlePass(int fd, std::string pass){
	if (pass.empty())
		sendCode(fd, "461", "", "Not enough parameters");
	else if (clients[fd]->get_auth() == true)
		sendCode(fd, "462", clients[fd]->get_nick(), ": You may not reregister");
	else if (_pass != pass)
		sendCode(fd, "464", clients[fd]->get_nick(), ": Password incorrect");
	else {
		clients[fd]->set_pass(pass);
		sendCode(fd, "338", clients[fd]->get_nick(), ": Password accepted");
	}
}
