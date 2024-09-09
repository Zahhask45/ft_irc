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
	//std::map<int , std::string>::iterator it = users.find(client.get_client_fd());
	/* if (it == users.end()){
	} */
		/* std::cout << client.get_client_fd() << std::endl;
    	std::pair<int, std::string> user_info(client.get_client_fd(), client.get_nick()); */
if (this->users.find(client.get_client_fd()) == this->users.end())
	{
		this->users.insert(std::pair<int, Client *>(client.get_client_fd(), &client));
	}
	//users.insert(std::pair<int, Client *>(client.get_client_fd(), &client));
}

void Channel::removeUser(std::string user_name){
	(void)user_name;
	/* if (users.find(user_name) != users.end())
		users.erase(user_name); */
}

std::string const &Channel::getName(void) const {return _name;}


void Channel::setName(std::string name){
	_name = name;
}


/* //TODO: CHANGE THIS
std::string Channel::getUser(std::string const user) const{
	std::map< std::string, std::pair<std::string,std::string> >::const_iterator it = users.find(user);
	if (it != users.end())
        return it->first;
    return std::string();
} */
