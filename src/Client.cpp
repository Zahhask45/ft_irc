#include "Client.hpp"

Client::Client(): _client_fd(), _user(), _nick() {
	_auth = false;
	_host = "GenericHost";
}

Client::Client(int fd): _client_fd(fd), _user(), _nick() {
	_auth = false;
	_host = "GenericHost";
}

Client::~Client(){
	// for(std::map<std::string, Channel *>::iterator it = this->channels.begin(); it != channels.end(); it++) {
	// 	delete it->second;
	// }
	// channels.clear();
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
