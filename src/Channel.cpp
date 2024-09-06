#include "Channel.hpp"

Channel::Channel(){}

Channel::Channel(const std::string &name): name(name){}

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

void Channel::addUser(client client){
	std::map< std::string, std::pair<std::string,std::string> >::iterator it = users.find(client.get_name());
	if (it == users.end()){
    	std::pair<std::string, std::pair<std::string, std::string> > user_info(client.get_name(), std::make_pair(client.get_nick(), client.get_pass()));
		users.insert(user_info);
	}
}

void Channel::removeUser(std::string user_name){
	if (users.find(user_name) != users.end())
		users.erase(user_name);
}

std::string const &Channel::getName(void) const {return name;}

//TODO: CHANGE THIS
std::string Channel::getUser(std::string const user) const{
	std::map< std::string, std::pair<std::string,std::string> >::const_iterator it = users.find(user);
	if (it != users.end())
        return it->first;
    return std::string();
}
