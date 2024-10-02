#include "Channel.hpp"

Channel::Channel(): _name(), users(){}

Channel::Channel(const std::string name): _name(name), users(){}

Channel::Channel(const std::string name, Client *Creator): _creator(Creator), _name(name), users() {
	//this->operators.insert(std::pair<int, Client *>(Creator->get_client_fd(), Creator));
	this->_creator->set_isOperator(true);
	this->_topic = " :Welcome to " + _name;
	this->_inviteChannel = false;
	this->addModes("+");
	this->_limit = 10;//alterar isto
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

void Channel::addUser(Client &client){
	if (this->users.find(client.get_client_fd()) == this->users.end())
			this->users.insert(std::pair<int, Client *>(client.get_client_fd(), &client));
}

void Channel::addOperator( Client &op ){
	if (this->operators.find(op.get_client_fd()) == this->operators.end())
		this->operators.insert(std::pair<int, Client *>(op.get_client_fd(), &op));
}

void Channel::removeUser(std::string user_name){
	std::map<int, Client *>::iterator it = users.begin();
	while (it != users.end()){
		if (it->second->get_nick() == user_name){
			users.erase(it);
			break;
		}
		it++;
	}
}

void Channel::removeOper(std::string oper){
	std::map<int, Client *>::iterator it = operators.begin();
	while (it != operators.end()){
		if (it->second->get_nick() == oper){
			operators.erase(it);
			break;
		}
		it++;
	}
}

std::string const &Channel::getName(void) const { return _name; }

std::string const &Channel::getTopic() const{ return _topic; }

std::string const &Channel::getKey() const {return _key;}

long unsigned int const &Channel::getLimit() const {return _limit;}

std::map<int, Client*>& Channel::getUsers() {return users;}

std::map<int, Client*> const & Channel::getOperators() const {return operators;}

std::map<int, Client*> const & Channel::getInviteList() const {return inviteList;}

bool const &Channel::getInviteChannel() const {return _inviteChannel;}

int Channel::getByName(std::string const &name) const {
	std::map<int, Client *>::const_iterator it = users.begin();
	while (it != users.end()){
		if (it->second->get_nick() == name)
			return it->first;
		it++;
	}
	return 0;
}

void Channel::setName(std::string const &name) {this->_name = name;}

void Channel::setUser(int const &id, Client *client) {
	std::map<int, Client *>::const_iterator it = users.find(id);
	if (it == users.end()) {
		users.insert(std::make_pair(id, client));
	}
}

void Channel::setTopic(std::string const &topic) {this->_topic = topic;}

void Channel::setInviteChannel(bool const &invitechannel) {this->_inviteChannel = invitechannel;}

void Channel::setKey(std::string const &key) {this->_key = key;}

void Channel::setLimit(int const &limit) {this->_limit = limit;}

std::string		Channel::listAllUsers() const {
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

void Channel::addInvite(int fd, Client *client) {
	if (this->inviteList.find(fd) == this->inviteList.end())
		this->inviteList.insert(std::pair<int, Client *>(fd, &client[fd]));
}

std::string Channel::getModes(){
	std::string modes;
	for (std::vector<std::string>::iterator it = _modes.begin(); it != _modes.end(); it++){
		modes += *it;
	}
	return modes;
}

void Channel::addModes(std::string mode){
	_modes.push_back(mode);
}

void Channel::removeModes(std::string mode){
	std::vector<std::string>::iterator it = std::find(_modes.begin(), _modes.end(), mode);
	if (it != _modes.end())
		_modes.erase(it);
}
