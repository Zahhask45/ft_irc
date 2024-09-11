#include "Client.hpp"

Client::Client(): _client_fd(), _name(), _nick() {
	_auth = false;
}
Client::Client(int newsocket): _client_fd(newsocket), _name(), _nick() {
	_auth = false;

}

Client::Client(Client &cp){
	*this = cp;
}

Client::~Client(){}

bool const &Client::get_auth() const{
	return _auth;
}

int const &Client::get_client_fd() const{
	return _client_fd;
}

std::string const &Client::get_name() const{
	return _name;
}
std::string const &Client::get_pass() const{
	return _pass;
}

std::string const &Client::get_nick() const{
	return _nick;
}

void Client::set_addr(struct sockaddr_storage value){
	_client_addr = value;
}

void Client::set_auth(bool value){
	_auth = value;
}

void Client::set_client_fd(int fd){
	_client_fd = fd;
}

void Client::set_user_info(char buffer[]){
	std::string bufferStr(buffer);
	size_t pos;

	// Procurar por USER
	pos = bufferStr.find("USER");
	if (pos != std::string::npos) {
	    // Extrair a informação do usuário e armazená-la no vetor
	    std::string line = bufferStr.substr(pos);
		pos = line.find(" ");
	    //std::string user = line.substr(pos + 1, line.find(' ', pos + 1) - pos - 1);
	    this->_name = line.substr(pos + 1, line.find(' ', pos + 1) - pos - 1);;
		// std::cout << "==========================================" << std::endl;
		// std::cout << _name << std::endl;
		// std::cout << "==========================================" << std::endl;
		// std::cout << line << std::endl;
		// std::cout << "==========================================" << std::endl;
	}

	// Procurar por PASS
	pos = bufferStr.find("PASS");
	if (pos != std::string::npos) {
	    // Extrair a informação da senha e armazená-la no vetor
		std::string line = bufferStr.substr(pos);
		pos = line.find(" ");
		_pass = line.substr(pos + 1, line.find('\n', pos + 1) - pos - 1 - 1);
		/* for(size_t i = 0; _pass[i] != '\0'; i++){
			std::cout << "123>" << _pass[i] << "<<<<<<" << std::endl;} */
	}

	// Procurar por NICK
	pos = bufferStr.find("NICK");
	if (pos != std::string::npos) {
	    // Extrair a informação do apelido e armazená-la no vetor
		std::string line = bufferStr.substr(pos);
		pos = line.find(" ");
		std::string nick = line.substr(pos + 1, line.find(' ', pos + 1) - pos - 1);
	    _nick = nick;
	}
}

void Client::addChannel(const std::string &channelName, Channel &channel){
	if (channels.find(channelName) == channels.end()){
		channels.insert(std::pair<std::string, Channel *>(channelName, &channel));
	}
}



