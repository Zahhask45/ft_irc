#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "colors.hpp"
class client;

class Channel
{
private:
    std::string name;
	//! CHANGE ALSO THIS FOR STRINGS AND NOT INT
    std::map< std::string, std::pair<std::string,std::string> > users;
    //TODO: BANNED USERS
public:
    Channel();
    Channel(const Channel &cp);
    Channel(const std::string &name);
	Channel &operator=(const Channel &orign);
    ~Channel();

    void addUser(client client);
    void removeUser(std::string user);
    std::string const &getName(void) const;
    std::set<int> const &getUsers(void) const;
	std::string getUser(std::string const user) const;
};

#include "client.hpp"

#endif
