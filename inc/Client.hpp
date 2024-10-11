#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"

//TODO: ADD STUFF FOR THE CLIENT USER

class Client
{
	private:
		//* STUFF ABOUT THE USER
		int _client_fd;
		std::string _user;
		std::string _nick;
		std::string _pass;
		std::string _host;
		std::string _mask;
		std::string _realname;
		bool _auth;
		bool _isOperator;
		char _buffer[1024];
		int bytes;

		struct sockaddr_storage _client_addr;
		socklen_t _client_len;
		std::map<std::string, Channel *> channels;

		Client(Client &cp);
		Client &operator=(Client &cp);
	public:
		Client();
		Client(int fd);
		~Client();


		//! CHANGE THIS TO PRIVATE, REMAKE THE FUCNTIONS WHERE THIS IS USED ON

	public: //GETTERS
		int const &get_client_fd() const;
		std::string const &getUser() const;
		std::string const &getPass() const;
		std::string const &getNick() const;
		bool const &getAuth() const;
		bool const &getIsOperator() const;
		std::string const &getHost() const;
		std::string const &getMask() const;
		std::string const &getRealname() const;
		char *getBuffer();
		int const &getBytes() const;

	public: //SETTERS
		void setAddr(struct sockaddr_storage value);
		void setClientFd(const int &fd);
		void setAuth(bool value);
		void setIsOperator(bool value);
		void setUser(const std::string &name);
		void setPass(const std::string &pass);
		void setNick(const std::string &nick);
		void setMask(const std::string &mask);
		void setRealname(const std::string &realname);
		void setBuffer(const char buffer[]);
		void setBytes(const int &bytes);
		void setBufferChar(size_t index, char value);

		void addChannel(const std::string &channelName, Channel &channel);
		void removeChannel(const std::string &channelName);
		void resetBytes();
		void resetBuffer();
};

#endif
