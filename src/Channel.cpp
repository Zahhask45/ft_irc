#include "Channel.hpp"

Channel::Channel(): _name(), users(){}

Channel::Channel(const std::string name): _name(name), users(){}

Channel::Channel(const std::string name, Client *Creator): _creator(Creator), _name(name), users() {
	//this->operators.insert(std::make_pair(Creator->get_client_fd(), Creator));
	//this->_creator->set_is_operator(true);
	this->add_operator(*Creator);
	this->_topic = ":Welcome to " + _name;
	this->_inviteChannel = false;
	this->_limit = 9999;//alterar isto
	this->_creationTime = time(NULL);
}

Channel::~Channel(){}

Channel &Channel::operator=(const Channel &origin) {
	if (this != &origin) {
		_name = origin._name;
		users = origin.users;
		operators = origin.operators;
	}
	return *this;
}

void Channel::add_user(Client &client){
	if (this->users.find(client.get_client_fd()) == this->users.end())
			this->users.insert(std::make_pair(client.get_client_fd(), &client));
}

void Channel::add_operator( Client &op ){
	if (this->operators.find(op.get_client_fd()) == this->operators.end())
		this->operators.insert(std::make_pair(op.get_client_fd(), &op));
}

void Channel::remove_user(std::string user_name){
	std::map<int, Client *>::iterator it = users.begin();
	while (it != users.end()){
		if (it->second->get_nick() == user_name){
			users.erase(it);
			break;
		}
		it++;
	}
}

void Channel::remove_oper(std::string oper){
	std::map<int, Client *>::iterator it = operators.begin();
	while (it != operators.end()){
		if (it->second->get_nick() == oper){
			it->second->set_isOperator(false);
			operators.erase(it);
			break;
		}
		it++;
	}
}

std::string const &Channel::get_name(void) const { return _name; }

std::string const &Channel::get_topic() const{ return _topic; }

std::string const &Channel::get_key() const {return _key;}

long unsigned int const &Channel::get_limit() const {return _limit;}

std::map<int, Client*>& Channel::get_users() {return users;}

std::map<int, Client*> const & Channel::get_operators() const {return operators;}

std::map<int, Client*> const & Channel::get_invite_list() const {return inviteList;}

bool const &Channel::get_invite_channel() const {return _inviteChannel;}

int Channel::get_by_name(std::string const &name) const {
	std::map<int, Client *>::const_iterator it = users.begin();
	while (it != users.end()){
		if (it->second->get_nick() == name)
			return it->first;
		it++;
	}
	return 0;
}

void Channel::set_name(std::string const &name) {this->_name = name;}

void Channel::set_user(int const &id, Client *client) {
	std::map<int, Client *>::const_iterator it = users.find(id);
	if (it == users.end()) {
		users.insert(std::make_pair(id, client));
	}
}

void Channel::set_topic(std::string const &topic) {this->_topic = topic;}

void Channel::set_invite_channel(bool const &invitechannel) {this->_inviteChannel = invitechannel;}

void Channel::set_key(std::string const &key) {this->_key = key;}

void Channel::set_limit(int const &limit) {this->_limit = limit;}

std::string		Channel::list_all_users() const {
	std::string		AllUsers(":");
	std::map<int, Client *>::const_iterator it = this->operators.begin();
	while (it != this->operators.end()){
		AllUsers.append("@" + it->second->get_nick() + " ");
		it++;
	}
	it = this->users.begin();
	while (it != this->users.end())
	{
		AllUsers.append(it->second->get_nick() + " ");
		it++;
	}
	return (AllUsers);
}

void Channel::add_invite(int fd, Client *client) {
	if (this->inviteList.find(fd) == this->inviteList.end())
		this->inviteList.insert(std::make_pair(fd, &client[fd]));
}

std::string Channel::get_modes(){
	std::string modes = "+";
	char elem[6] = { 'i', 'k', 'l', 'o', 't'};

	for (int i = 0; i < 6; i++){
		for (std::vector<char>::iterator it = _modes.begin(); it != _modes.end(); it++){
			if (*it == elem[i])
				modes += *it;
		} 
	}
	if (_key != "")
	{
		if (modes.find('l') != std::string::npos){
			
			modes += " " + _key + " :" + toString(_limit);
		}
		else
			modes += " :" + _key;
	}
	else if (modes.find('l') != std::string::npos){
		modes += " :" + toString(_limit);
	}
	return modes;
}

bool Channel::add_modes(char mode){
	for(std::vector<char>::iterator it = _modes.begin(); it != _modes.end(); it++){
		if (*it == mode)
			return false;
	}
	_modes.push_back(mode);
	return true;
}

bool Channel::remove_modes(char mode){
	std::vector<char>::iterator it = std::find(_modes.begin(), _modes.end(), mode);
	if (it != _modes.end()){
		_modes.erase(it);
		return true;
	}
	return false;
}

int Channel::get_creator_fd() const {
	return _creator->get_client_fd();
}
