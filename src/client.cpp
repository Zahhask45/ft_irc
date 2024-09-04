#include "client.hpp"

client::client(): _client_fd(), _name(), _nick(){}

client::~client(){}

int const &client::get_socket() const{
	return _socket;
}

void client::set_socket(int value){
	_socket = value;
}

void client::set_addr(struct sockaddr_in value){
	_client_addr = value;
}

