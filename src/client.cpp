#include "client.hpp"

client::client(): _client_fd(), _name(), _nick(){}

client::client(client &cp){
	*this = cp;
}


client::~client(){}

int const &client::get_socket() const{
	return _socket;
}

int const &client::get_client_fd() const{
	return _client_fd;
}

std::string const &client::get_name() const{
	return _name;
}
std::string const &client::get_pass() const{
	return _pass;
}

std::string const &client::get_nick() const{
	return _nick;
}

void client::set_socket(int value){
	_socket = value;
}

void client::set_addr(struct sockaddr_in value){
	_client_addr = value;
}

void client::set_client_fd(int fd){
	_client_fd = fd;
}

void client::set_user_info(char buffer[]){
	std::string bufferStr(buffer);
	size_t pos;

	// Procurar por USER
	pos = bufferStr.find("USER");
	if (pos != std::string::npos) {
	    // Extrair a informação do usuário e armazená-la no vetor
	    std::string user = bufferStr.substr(pos);
	    _name = user;
	}

	// Procurar por PASS
	pos = bufferStr.find("PASS");
	if (pos != std::string::npos) {
	    // Extrair a informação da senha e armazená-la no vetor
	    std::string pass = bufferStr.substr(pos);
	    _pass = pass;
	}

	// Procurar por NICK
	pos = bufferStr.find("NICK");
	if (pos != std::string::npos) {
	    // Extrair a informação do apelido e armazená-la no vetor
	    std::string nick = bufferStr.substr(pos);
	    _nick = nick;
	}
}
