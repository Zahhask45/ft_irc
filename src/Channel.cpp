#include "Channel.hpp"

Channel::Channel(): name(), users(){}

Channel::Channel(const std::string name): name(name), users(){}

Channel::Channel(const Channel &cp){
    *this = cp;
}

Channel::~Channel(){}

Channel &Channel::operator=(const Channel &origin) {
	if (this != &origin) {
		name = origin.name;
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

std::string const &Channel::getName(void) const {return name;}

/* //TODO: CHANGE THIS
std::string Channel::getUser(std::string const user) const{
	std::map< std::string, std::pair<std::string,std::string> >::const_iterator it = users.find(user);
	if (it != users.end())
        return it->first;
    return std::string();
} */

void Channel::setName(std::string name) {this->name = name;}

void Channel::setUser(int id, Client *client) {
    std::map<int, Client *>::iterator it = users.find(id);
    if (it == users.end()) {
        users.insert(std::make_pair(id, client));
    }
}
