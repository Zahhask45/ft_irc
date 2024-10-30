#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"


class Client {
	private:
		int _client_fd;
		std::string _user;
		std::string _nick;
		std::string _pass;
		std::string _host;
		std::string _mask;
		std::string _realname;
		time_t last_ping_time;
		bool _auth;
		bool _isOperator;
		bool _flagNick;

		struct sockaddr_storage _client_addr;
		socklen_t _client_len;
		std::map<std::string, Channel *> channels;

		std::string _buffer;
		int bytes_received;

		Client(Client &cp);
		Client &operator=(Client &cp);
	public:
		Client();
		Client(int fd);
		~Client();

		int ready_in;

	public: //GETTERS
		int const &get_client_fd() const;
		std::string const &get_user() const;
		std::string const &get_pass() const;
		std::string const &get_nick() const;
		bool const &get_auth() const;
		bool const &get_isOperator() const;
		bool const &get_flagNick() const;
		std::string const &get_host() const;
		std::string const &get_mask() const;
		std::string const &get_realname() const;
		const std::string get_buffer() const;
		const std::string get_first_buffer();
		int const &get_bytes_received() const;

	public: //SETTERS
		void set_addr(struct sockaddr_storage value);
		void set_client_fd(const int &fd);
		void set_auth(bool value);
		void set_isOperator(bool value);
		void set_user(const std::string &name);
		void set_pass(const std::string &pass);
		void set_nick(const std::string &nick);
		void set_mask(const std::string &mask);
		void set_realname(const std::string &realname);
		void set_buffer(const char* buffer);
		void set_bytes_received(const int value);
		void set_flagNick(bool value);

		void clean_buffer();
		void add_to_buffer(const char* buffer);
		void add_to_buffer(const std::string buffer);

		void add_channel(const std::string &channelName, Channel &channel);
		void remove_channel(const std::string &channelName);

		bool empty_buffer();
};

#endif
