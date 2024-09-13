#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "colors.hpp"
class Client;

class Channel
{
private:
    std::string _name;
	//! CHANGE ALSO THIS FOR STRINGS AND NOT INT
    std::map<int, Client *> users;
    //TODO: BANNED USERS
public:
    Channel();
    Channel(const Channel &cp);
    Channel(const std::string name);
	Channel &operator=(const Channel &orign);
    ~Channel();

    void addUser(Client &client);
    void removeUser(std::string user);
    std::string const &getName(void) const;
    std::map<int, Client *> getUsers(void) const;
	std::string getUser(std::string const user) const;

	void setName(std::string name);

	std::string listAllUsers() const;
};

#include "Client.hpp"

#endif
