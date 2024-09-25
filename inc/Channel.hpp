#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "colors.hpp"
class Client;

class Channel
{
private:

    Client *    _creator;
    std::string _name;
	//! CHANGE ALSO THIS FOR STRINGS AND NOT INT
    std::map<int, Client *> users;
    std::map<int, Client *>	operators;
    //TODO: BANNED USERS
    
    Channel(const Channel &cp);
    
	Channel &operator=(const Channel &orign);
public:
    Channel();
    Channel(const std::string name);
    Channel(const std::string name, Client *Creator);
    ~Channel();

    void addUser(Client &client);
	void addOperator( Client &op );
    void removeUser(std::string user);
    void removeOper(std::string oper);
    void setName(std::string const &name);
    void setUser(int const &id, Client *client);
    
    std::string	listAllUsers() const;
    
    std::string const &getName(void) const;
	std::map<int, Client*>& getUsers();
    std::map<int, Client *>	const &getOperators() const;

};

#include "Client.hpp"

#endif
