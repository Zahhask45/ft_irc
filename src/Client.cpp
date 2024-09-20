#include "Client.hpp"

Client::Client(): _client_fd(), _user(), _nick(), _host("banana") {
	_auth = false;
	_host = "GenericHost";
}
Client::Client(int newsocket): _client_fd(newsocket), _user(), _nick(), _host("banana") {
	_auth = false;

}
Client::Client(int fd): _client_fd(fd), _user(), _nick(), _host("banana") {
	_auth = false;

}

Client::~Client(){
	close(get_client_fd());

}

bool const &Client::get_auth() const{ return _auth; }

int const &Client::get_client_fd() const{ return _client_fd; }

std::string const &Client::get_user() const{ return _user; }

std::string const &Client::get_pass() const{ return _pass; }

std::string const &Client::get_nick() const{ return _nick; }

std::string const &Client::get_host() const { return _host; }

std::string const &Client::get_mask() const{ return _mask; }

void Client::set_addr(struct sockaddr_storage value){
	_client_addr = value;
}

void Client::set_auth(bool value){
	this->_auth = value;
}

void Client::set_client_fd(int const &fd){
	this->_client_fd = fd;
}

void Client::set_user(std::string user){
	_user = user;
}

void Client::set_nick(std::string nick){
	_nick = nick;
}

void Client::set_pass(std::string pass){
	_pass = pass;
}

//? CAN BE DIFFERENT
void Client::set_mask(std::string mask){
	_mask = mask;
}









void Client::set_user(std::string const &user){
	this->_user = user;	
}

void Client::set_nick(std::string const &nick){
	this->_nick = nick;
}

void Client::set_pass(std::string const &pass){
	this->_pass = pass;
}

void Client::set_mask(std::string const &mask) {
	this->_mask = mask;
}

void Client::addChannel(const std::string &channelName, Channel &channel){
	if (channels.find(channelName) == channels.end()){
		channels.insert(std::pair<std::string, Channel *>(channelName, &channel));
	}
}

void Client::removeChannel(const std::string &channelName){
	if (channels.find(channelName) != channels.end()){
		channels.erase(channelName);
	}
}

void Client::addChannel(const std::string &channelName, Channel &channel){
	if (channels.find(channelName) == channels.end()){
		channels.insert(std::pair<std::string, Channel *>(channelName, &channel));
	}
}



