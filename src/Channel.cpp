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

void Channel::addUser(int user){
	if (users.find(user) == users.end())
    	users.insert(user);
}

void Channel::removeUser(int user){
    users.erase(user);
}

std::string const &Channel::getName(void) const {return name;}
std::set<int> const &Channel::getUsers(void) const {return users;}

//TODO: CHANGE THIS
int Channel::getUser(int user_fd){
	if (users.find(user_fd) != users.end())
		return user_fd;
	return 0;
}
