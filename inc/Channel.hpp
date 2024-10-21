#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "colors.hpp"
class Client;

class Channel
{
private:

    Client *    _creator;
    std::string _name;
    std::string _topic;
    std::string _key;
    long unsigned int         _limit;
    int         _creationTime;
    bool        _inviteChannel;
	//! CHANGE ALSO THIS FOR STRINGS AND NOT INT
    std::map<int, Client *> users;
    std::map<int, Client *>	operators;
    std::map<int, Client *>	inviteList;
	std::vector<char> _modes;

    //TODO: BANNED USERS
    
    Channel(const Channel &cp);
    
	Channel &operator=(const Channel &orign);
public:
    Channel();
    Channel(const std::string name);
    Channel(const std::string name, Client *Creator);
    ~Channel();

    std::string const &get_name(void) const;
    std::string const &get_topic(void) const;
    std::string const &get_key(void) const;
    long unsigned int const &get_limit(void) const;
    bool const &get_invite_channel(void) const;
	std::string get_modes();
    int const &get_creation_time() const;

    std::map<int, Client*> &get_users();
    std::map<int, Client *>	const &get_operators() const;
    std::map<int, Client *>	const &get_invite_list() const;

    void add_user(Client &client);
	void add_operator( Client &op );
    
	bool add_modes(char mode);
	bool remove_modes(char mode);

    void remove_user(std::string user);
    void remove_oper(std::string oper);
    
    void set_name(std::string const &name);
    void set_user(int const &id, Client *client);
    void set_topic(std::string const &topic);
    void set_invite_channel(bool const &invite);
    void set_key(std::string const &key);
    void set_limit(int const &limit);
    
    std::string	list_all_users() const;
    
    int get_by_name(std::string const &name) const;

    void add_invite(int fd, Client *client);
    int get_creator_fd() const;
};

#include "Client.hpp"

#endif
