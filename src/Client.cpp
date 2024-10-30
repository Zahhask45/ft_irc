#include "Client.hpp"

Client::Client(): _client_fd(), _user(), _nick(), bytes_received(0) {
	_auth = false;
	_isOperator = false;
	_host = "Terracotta";
	ready_in = 1;
	_flagNick = false;
}

Client::Client(int fd): _client_fd(fd), _user(), _nick(), bytes_received(0) {
	_auth = false;
	_isOperator = false;
	_host = "Terracotta";
	ready_in = 1;
	_flagNick = false;

}

Client::~Client() {}

bool const &Client::get_auth() const{ return _auth; }

bool const &Client::get_isOperator() const{ return _isOperator; }

int const &Client::get_client_fd() const{ return _client_fd; }

std::string const &Client::get_user() const{ return _user; }

std::string const &Client::get_pass() const{ return _pass; }

std::string const &Client::get_nick() const{ return _nick; }

std::string const &Client::get_host() const { return _host; }

std::string const &Client::get_mask() const{ return _mask; }

std::string const &Client::get_realname() const{ return _realname; }

const std::string Client::get_buffer() const{ return _buffer; }

const std::string Client::get_first_buffer() {

	std::istringstream commandStream(_buffer);
	std::string line;
	if (_buffer.find("\n") == std::string::npos) {
		return "";
	}
	std::getline(commandStream, line, '\n');
	size_t pos = line.size();
	_buffer.erase(0, pos + 1);
	return line;
}

int const &Client::get_bytes_received() const{ return bytes_received; }

bool const &Client::get_flagNick() const{ return _flagNick; }

void Client::set_addr(struct sockaddr_storage value) {
	_client_addr = value;
}

void Client::set_auth(bool value) {
	this->_auth = value;
}

void Client::set_isOperator(bool value) {
	this->_isOperator = value;
}

void Client::set_client_fd(int const &fd) {
	this->_client_fd = fd;
}

void Client::set_user(const std::string &user) {
	this->_user = user;
}

void Client::set_nick(const std::string &nick) {
	this->_nick = nick;
}

void Client::set_pass(const std::string &pass) {
	this->_pass = pass;
}

void Client::set_mask(const std::string &mask) {
	this->_mask = mask;
}

void Client::set_realname(std::string const &realname) {
	this->_realname = realname;
}

void Client::set_buffer(const char* buffer) {
	_buffer = buffer;
}

void Client::set_bytes_received(int value) {
	this->bytes_received = value;
}

void Client::set_flagNick(bool value) {
	this->_flagNick = value;
}

void Client::add_channel(const std::string &channelName, Channel &channel) {
	if (channels.find(channelName) == channels.end()) {
		channels.insert(std::make_pair(channelName, &channel));
	}
}

void Client::remove_channel(const std::string &channelName) {
	if (channels.find(channelName) != channels.end()) {
		channels.erase(channelName);
	}
}

void Client::clean_buffer() {
	_buffer.erase();
}

void Client::add_to_buffer(const char* append) {
	_buffer += append;
}

void Client::add_to_buffer(const std::string append) {
	std::istringstream appendStream(append);
	std::string line;
	while (std::getline(appendStream, line, '\n')) {
		std::istringstream iss(line);
		std::string cmd;
		iss >> cmd;
		_buffer += cmd;
	}
}

bool Client::empty_buffer() {
	if (_buffer.empty())
		return 1;
	return 0;
}
