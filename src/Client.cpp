#include "Client.hpp"

Client::Client(): _client_fd(), _name(), _nick() {
	_auth = false;
}

// Client::Client(Client &cp){
// 	*this = cp;
// }

Client::~Client(){}

int const &Client::get_socket() const{
	return _socket;
}

bool const &Client::get_auth() const{
	return _auth;
}

int const &Client::get_client_fd() const{
	return _client_fd;
}

std::string Client::get_host() const {
    return std::string(inet_ntoa(_client_addr.sin_addr));
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

void Client::set_socket(int value){
	_socket = value;
}

void Client::set_addr(struct sockaddr_in value){
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
        this->_name = extract_value(line, "USER");
		// std::cout << "start>>" << this->_name << "<<end\n" << std::endl;
    }

	// Procurar por PASS
	pos = bufferStr.find("PASS");
    if (pos != std::string::npos) {
	    // Extrair a informação da senha e armazená-la no vetor
        std::string line = bufferStr.substr(pos);
        this->_pass = extract_value(line, "PASS");
		// std::cout << "start>>" << this->_pass << "<<end\n" << std::endl;
    }

	// Procurar por NICK
	pos = bufferStr.find("NICK");
    if (pos != std::string::npos) {
	    // Extrair a informação do apelido e armazená-la no vetor
        std::string line = bufferStr.substr(pos);
        this->_nick = extract_value(line, "NICK");
		// std::cout << "start>>" << this->_nick << "<<end\n" << std::endl;
    }
}

std::string Client::extract_value(const std::string& line, const std::string& key) {
    size_t start = line.find(key) + key.length();  // Find end of key
    if (start == std::string::npos) {
        return "";  // Key not found
    }
    // Skip any spaces or colons that follow the key
    while (start < line.length() && (line[start] == ' ' || line[start] == ':')) {
        start++;
    }
    // Find the end of the first word (next space or newline)
    size_t end = line.find_first_of(" \n", start);
    if (end == std::string::npos) {
        end = line.length();  // If no space or newline, take until the end of the line
    }
    std::string value = line.substr(start, end - start);

    // Remove any trailing newline character
    if (!value.empty() && value[value.length() - 1] == '\n') {
        value = value.substr(0, value.length() - 1);
    }

    return value;
}
