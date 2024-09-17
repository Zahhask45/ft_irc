#include "Channel.hpp"

Channel::Channel(): _name(), users(){}

Channel::Channel(const std::string name): _name(name), users(){}

Channel::Channel(const Channel &cp){
	*this = cp;
}

Channel::~Channel(){}

Channel &Channel::operator=(const Channel &origin) {
	if (this != &origin) {
		_name = origin._name;
		users = origin.users;
	}
	return *this;
}

void Channel::addUser(Client &client){
	if (this->users.find(client.get_client_fd()) == this->users.end())
		{
			this->users.insert(std::pair<int, Client *>(client.get_client_fd(), &client));
		}
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

std::string const &Channel::getName(void) const {return _name;}

std::map<int, Client*>& Channel::getUsers() {
        return users;
    }
/* //TODO: CHANGE THIS
	std::map< std::string, std::pair<std::string,std::string> >::const_iterator it = users.find(user);
	if (it != users.end())
		return it->first;
	return std::string();
} */

void Channel::setName(std::string name) {this->_name = name;}

void Channel::setUser(int id, Client *client) {
	std::map<int, Client *>::iterator it = users.find(id);
	if (it == users.end()) {
		users.insert(std::make_pair(id, client));
	}
}

std::string		Channel::listAllUsers() const {
	std::string		AllUsers(" :");
	std::map<int, Client *>::const_iterator it = this->users.begin();
	
	it = this->users.begin();
	while (it != this->users.end())
	{
		AllUsers.append(it->second->get_nick() + " ");
		it++;
	}
	return (AllUsers);
}