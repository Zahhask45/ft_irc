#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "colors.hpp"

class Channel
{
private:
    std::string name;
	//! CHANGE ALSO THIS FOR STRINGS AND NOT INT
    std::set<int> users;
    //TODO: BANNED USERS
public:
    Channel();
    Channel(const Channel &cp);
    Channel(const std::string &name);
	Channel &operator=(const Channel &orign);
    ~Channel();

    void addUser(int userFd);
    void removeUser(int userFd);
    std::string const &getName(void) const;
    std::set<int> const &getUsers(void) const;
	int getUser(int user_fd);
};

#endif
