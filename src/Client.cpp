#include "Client.hpp"

Client::Client(): _client_fd(), _user(), _nick(), bytes(0) {
	_auth = false;
	_isOperator = false;
	_host = "Terracota";
	bytes = 0;
}

Client::Client(int fd): _client_fd(fd), _user(), _nick(), bytes(0) {
	_auth = false;
	_isOperator = false;
	_host = "Terracota";

	bytes = 0;
}

Client::~Client(){}

bool const &Client::getAuth() const{ return _auth; }

bool const &Client::getIsOperator() const{ return _isOperator; }

// bool const &Client::get_flagNick() const{ return _flagNick; }

int const &Client::get_client_fd() const{ return _client_fd; }

std::string const &Client::getUser() const{ return _user; }

std::string const &Client::getPass() const{ return _pass; }

std::string const &Client::getNick() const{ return _nick; }

std::string const &Client::getHost() const { return _host; }

std::string const &Client::getMask() const{ return _mask; }

std::string const &Client::getRealname() const{ return _realname; }

char *Client::getBuffer() { return _buffer; }

// int const &Client::getBytes() const{ return bytes; }

void Client::setAddr(struct sockaddr_storage value){
	_client_addr = value;
}

void Client::setAuth(bool value){
	this->_auth = value;
}

void Client::setIsOperator(bool value){
	this->_isOperator = value;
}

void Client::setClientFd(int const &fd){
	this->_client_fd = fd;
}

void Client::setUser(const std::string &user){
	this->_user = user;
}

void Client::setNick(const std::string &nick){
	this->_nick = nick;
}

void Client::setPass(const std::string &pass){
	this->_pass = pass;
}

void Client::setMask(const std::string &mask){
	this->_mask = mask;
}

void Client::setRealname(std::string const &realname){
	this->_realname = realname;
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
