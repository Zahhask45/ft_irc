#include "Server.hpp"

static std::string serverTimestamp(){
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
	return buf;
}

void Server::handleAuth(int fd){
	if (clients[fd] && (clients[fd]->get_user().empty() 
		|| clients[fd]->get_pass().empty() 
		|| clients[fd]->get_nick().empty())){
		sendCode(fd, "461", "", "Not enough parameters"); //ERR_NEEDMOREPARAMS
		return;
	}
	if (clients[fd]->get_auth() == true){
		sendCode(fd, "462", clients[fd]->get_nick(), ": You may not reauth"); // ERR_ALREADYREGISTRED
		return;
	}
	if (clients[fd]->get_pass().compare(_pass) == 0){
		sendCode(fd, "001", clients[fd]->get_nick(), ":Welcome to the 42Porto IRC Network " + clients[fd]->get_mask());
		sendCode(fd, "002", clients[fd]->get_nick(), ":Your host is " + clients[fd]->get_host() + ", running version 1.0");
		sendCode(fd, "003", clients[fd]->get_nick(), ":This server was created " + serverTimestamp());
		sendCode(fd, "004", clients[fd]->get_nick(), clients[fd]->get_host() + " InspIRCd-3 BDHIORSTWcdghikorswxz ACIJKMNOPQRSTYabceghiklmnopqrstvz :IJYabeghkloqv"); //! Explicacao no arquivo explain.txt
		sendCode(fd, "005", clients[fd]->get_nick(), "CHANMODES=Ibeg,k,Jl,ACKMNOPQRSTiprstz :are supported by this server");
		sendCode(fd, "371", clients[fd]->get_nick(), ":User is Authenticated");
		sendCode(fd, "375", clients[fd]->get_nick(), ":" + clients[fd]->get_host() + " Message of the day");
		sendCode(fd, "372", clients[fd]->get_nick(), ":    ▟██▛╗██▛███   ");
		sendCode(fd, "372", clients[fd]->get_nick(), ":  ▟██▛╔═╝█▛ ▟█▛╗");
		sendCode(fd, "372", clients[fd]->get_nick(), ":▟██▛╔═╝   ╚▟▛╔═╝                       ▝▜▉▛▘█▀▜▉▐▛▀▜");
		sendCode(fd, "372", clients[fd]->get_nick(), ":████████╗▟█▛ ▟█╗ █▀█ █▀█ █▀█ ▀█▀ █▀█    ▐▉▌ █▄▟▀▐▌");
		sendCode(fd, "372", clients[fd]->get_nick(), ": ╚═══███║███▟██║ █▀▀ █▄█ █▀▄  █  █▄█ ▁▁▗▟▉▙▖▉ ▐▙▐▙▄▟");
		sendCode(fd, "372", clients[fd]->get_nick(), ":     ███║ ╚════╝ ");
		sendCode(fd, "372", clients[fd]->get_nick(), ":      ╚═╝			By: bmonteir; jodos-sa; mamaral-");
		sendCode(fd, "376", clients[fd]->get_nick(), ":End of message of the day.");
		sendCode(fd, "374", clients[fd]->get_nick(), ":Now you can join channels and chat with other users");
		sendCode(fd, "396", clients[fd]->get_nick(), clients[fd]->get_host() + " :is now your displayed host");
		clients[fd]->set_auth(true);
		return ;
	}
	else
		sendCode(fd, "372", clients[fd]->get_nick(), ": User is not Authenticated");
}

//! PASS should come before NICK and USER, maybe we should create a function to force the others commands to come only after PASS.
void Server::handlePass(int fd, std::istringstream &command){
	std::string pass;
	command >> pass;
	pass = extract_value(pass);
	if (pass.empty())
		sendCode(fd, "461", "", "Not enough parameters"); //ERR_NEEDMOREPARAMS
	else if (clients[fd]->get_auth() == true)
		sendCode(fd, "462", clients[fd]->get_nick(), ": You may not reregister"); // ERR_ALREADYREGISTRED
	else if (_pass != pass)
		sendCode(fd, "464", clients[fd]->get_nick(), ": Password incorrect");
	else {
		clients[fd]->set_pass(pass);
		sendCode(fd, "338", clients[fd]->get_nick(), ": Password accepted");
	}
}

void Server::handleNick(int fd, std::istringstream &command){
	std::string nick;
	command >> nick;
	
	if (nick.empty()){
		sendCode(fd, "431", "", "No nickname given"); // ERR_NONICKNAMEGIVEN
		clients[fd]->set_nick("\0");
		return;
	}
	std::map<int, Client *>::iterator it;
	for(it = clients.begin(); it != clients.end(); it++){
		if (it->second->get_nick() == nick){
			sendCode(fd, "433", nick, ":Nickname is already in use");
			this->clients[fd]->set_nick(nick);
			return;
		}
	}
	if (this->clients[fd]->get_nick().empty())
		this->clients[fd]->set_nick(nick);
	else{
		std::string changeNick = ":" + this->clients[fd]->get_nick() + " NICK " + nick + "\r\n";
		std::string nickChangeMsg = this->clients[fd]->get_mask() + "NICK :" + nick + "\r\n";
		print_client(fd, changeNick);
		this->clients[fd]->set_nick(nick);
		_ToAll(fd, nickChangeMsg);
	}
	clients[fd]->set_mask(":" + clients[fd]->get_nick() + "!" + clients[fd]->get_user() + "@" + clients[fd]->get_host() + " ");
}

void Server::handleUser(int fd, std::istringstream &command){
	std::string username, hostname, servername, realname;
	command >> username >> hostname >> servername; 
	std::getline(command, realname);
	if (username.empty() || hostname.empty() || servername.empty() || realname.empty()){
		sendCode(fd, "461", "", "Not enough parameters"); // ERR_NEEDMOREPARAMS
		return;
	}
	if (clients[fd]->get_user().empty()){
		this->clients[fd]->set_user(username);
		this->clients[fd]->set_realname(realname.substr(2, realname.size() - 3));
		if (clients[fd]->get_nick() != "\0")
			clients[fd]->set_mask(":" + clients[fd]->get_nick() + "!" + clients[fd]->get_user() + "@" + clients[fd]->get_host() + " ");
	}
	else{
		sendCode(fd, "462", clients[fd]->get_nick(), ": You may not reregister"); // ERR_ALREADYREGISTRED
	}
}

void Server::handleJoin(int fd, std::istringstream &command){
	std::string channelName, key, line;
	command >> line >> key;
	std::stringstream ss(line);

	if (clients[fd]->get_auth() == false){
		print_client(fd, "Need to Auth the user\n");
		sendCode(fd, "451", clients[fd]->get_nick(), ": You have not registered"); // ERR_NOTREGISTERED
		return ;
	}
	if (line.empty()){
		sendCode(fd, "461", "", "Not enough parameters"); // ERR_NEEDMOREPARAMS
		return ;
	}

	while (std::getline(ss, channelName, ',')){
		if (channelName[0] != '#' || channelName == "#"){
			sendCode(fd, "476", clients[fd]->get_nick(), channelName + " :Invalid channel name"); // ERR_BADCHANMASK
			return ;
		}
		if (get_channel(channelName) == NULL)
			create_channel(channelName, fd);
		if ((this->channels[channelName]->get_invite_channel() == true 
		&& this->channels[channelName]->get_invite_list().find(fd) == this->channels[channelName]->get_invite_list().end())
		&& this->channels[channelName]->get_operators().find(fd) == this->channels[channelName]->get_operators().end()){
			sendCode(fd, "473", clients[fd]->get_nick(), channelName + " :Cannot join channel (+i) invite only"); // ERR_INVITEONLYCHAN
			return ;
		}
		if (!this->channels[channelName]->get_key().empty() && this->channels[channelName]->get_key() != key){
			sendCode(fd, "475", clients[fd]->get_nick(), channelName + " :Cannot join channel (+k) bad key"); // ERR_BADCHANNELKEY
			return ;
		}
		if (this->channels[channelName]->get_users().size() >= this->channels[channelName]->get_limit() ){
			sendCode(fd, "471", clients[fd]->get_nick(), channelName + " :Cannot join channel (+l) limit reached"); // ERR_CHANNELISFULL
			return ;
		}

		this->clients[fd]->add_channel(channelName, *this->channels[channelName]);
		
		this->channels[channelName]->add_user(get_client(fd));
		
		print_client(fd, clients[fd]->get_mask() + "JOIN :" + channelName + "\r\n");

		sendCode(fd, "332", clients[fd]->get_nick(), channelName + " " + this->channels[channelName]->get_topic());
		sendCode(fd, "353", clients[fd]->get_nick() + " = " + channelName, this->channels[channelName]->list_all_users());
		sendCode(fd, "366", clients[fd]->get_nick(), channelName + " :End of /NAMES list");
		_ToAll(this->channels[channelName], fd, "JOIN :" + channelName + "\r\n");
	}
}

void sendReply(int fd, const std::string &reply){
	size_t total = 0;
	while (total != reply.length()){
		ssize_t nb = ::send(fd, reply.c_str() + total, reply.length() - total, 0);
		if (nb == -1)
			std::cout << "send error" << std::endl;
		total += nb;
	}
}


void Server::handlePrivmsg(int fd, std::istringstream &command){
	std::string target, message;
	command >> target;
	std::getline(command, message);
	if (target.empty() || message.empty()){
		sendCode(fd, "411", clients[fd]->get_nick(), ": No recipient given (PRIVMSG)");
		return ;
	}
	if (target[0] == '#'){
		if (channels.find(target) == channels.end()){
			sendCode(fd, "401", clients[fd]->get_nick(), target + " :No such nick/channel");
			return ;
		}
		if (channels[target]->get_users().find(fd) == channels[target]->get_users().end()){
			sendCode(fd, "404", clients[fd]->get_nick(), target + " :Cannot send to channel");
			return ;
		}
		_ToAll(channels[target], fd, "PRIVMSG " + target + " " + message + "\n");
	}
	else{
		int receiver_fd = 0;
		std::map<int, Client *>::iterator it = this->clients.begin();
		
		while(it != this->clients.end())
		{
			if (it->second->get_nick() == target){
					receiver_fd = it->second->get_client_fd();
				break;
			}
			it++;
		}
		if (receiver_fd == 0) {
			sendCode(fd, "401", clients[fd]->get_nick(), target + " :No such nick/channel"); // ERR_NOSUCHNICK
			return;
        }
			if (message.find(toString("\x01") + "DCC SEND") != std::string::npos) {
			handleAcceptFile(fd, message, target);
			sendReply(receiver_fd, clients[fd]->get_mask() + "PRIVMSG " + target +  message + "\n");
		}
		// _sendall(receiver_fd, clients[fd]->get_mask() + "PRIVMSG " + target + " " + message + "\n");
		else if(receiver_fd)
			print_client(receiver_fd, clients[fd]->get_mask() + "PRIVMSG " + target + " " + message + "\n");
	}
}

void Server::handlePart(int fd, std::istringstream &command){
	std::string channelName;
	command >> channelName;
	if (channelName.empty()){
		print_client(fd, "Channel name is empty\n");
		return ;
	}
	if (channels.find(channelName) == channels.end()){
		print_client(fd, "Channel does not exist\n");
		return ;
	}
	Channel *channel = channels[channelName];
	if (channel->get_users().find(fd) == channel->get_users().end()){
		print_client(fd, "You are not in this channel\n");
		return ;
	}
	print_client(fd, clients[fd]->get_mask() + "PART " + channelName + "\r\n");
	_ToAll(channel, fd, "PART " + channelName + "\r\n");
	channel->remove_user(clients[fd]->get_nick());
	channel->remove_oper(clients[fd]->get_nick());
	if (this->channels[channelName]->list_all_users() == ":"){
		delete this->channels[channelName];
		this->channels.erase(channelName);
	}
	clients[fd]->remove_channel(channelName);
}

void Server::handleQuit(int fd, std::istringstream &command){
	std::string message;
	command >> message;
	std::string response = clients[fd]->get_mask() + "QUIT :" + message + "\r\n";
	_ToAll(fd, response);
	for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); it++){
		if (it->second->get_users().find(fd) != it->second->get_users().end()){
			it->second->remove_user(clients[fd]->get_nick());
			it->second->remove_oper(clients[fd]->get_nick());
			clients[fd]->remove_channel(it->first);
		}
	}
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, clients[fd]->get_client_fd(), NULL) == -1) {
		std::cerr << "Error removing socket from epoll(quit): " << strerror(errno) << std::endl;
	}
	close(clients[fd]->get_client_fd());
	end_connection(fd);
	// delete clients[fd];
	// this->clients.erase(fd);
	// this->_cur_online--;
	this->_events[fd].data.fd = this->_events[this->_cur_online].data.fd;
	print_client(fd, response);
}

void Server::handlePing(int fd, std::istringstream &command){
	std::string server;
	command >> server;
	if (server.empty()){
		sendCode(fd, "409", "", "No origin specified");
		return ;
	}
	sendCode(fd, "PONG", clients[fd]->get_host(), server);
}

void Server::handleWho(int fd, std::istringstream &command){
	std::string mask;
	command >> mask;
	if (mask.empty()){
		sendCode(fd, "461", "", "Not enough parameters");
		return ;
	}
	std::map<int, Client *>::iterator it = clients.begin();
	while (it != clients.end()){
		if (it->second->get_mask().find(mask) != std::string::npos){
			sendCode(fd, "352", clients[fd]->get_nick(), mask + " " + it->second->get_user() + " " + it->second->get_host() + " " + it->second->get_host() + " " + it->second->get_nick() + " H :0 " + it->second->get_realname());
		}
		it++;
	}
	sendCode(fd, "315", clients[fd]->get_nick(), mask + " :End of /WHO list");
}

void Server::handleWhois(int fd, std::istringstream &command){
	std::string mask;
	command >> mask;
	if (mask.empty()){
		sendCode(fd, "461", "", "Not enough parameters");
		return ;
	}
	std::map<int, Client *>::iterator it = clients.begin();
	while (it != clients.end()){
		if (it->second->get_mask().find(mask) != std::string::npos){
			sendCode(fd, "311", clients[fd]->get_nick(), mask + " " + it->second->get_user() + " " + it->second->get_host() + " * :" + it->second->get_realname());
			sendCode(fd, "312", clients[fd]->get_nick(), mask + " " + it->second->get_host() + " :42Porto");
			sendCode(fd, "319", clients[fd]->get_nick(), mask + " :");
		}
		it++;
	}
	sendCode(fd, "318", clients[fd]->get_nick(), mask + " :End of /WHOIS list");
}

void Server::handleList(int fd){
	for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); it++){
		sendCode(fd, "322", clients[fd]->get_nick(), it->first + " " + toString(it->second->get_users().size()) + " :" + it->second->get_topic());
	}
	sendCode(fd, "323", clients[fd]->get_nick(), ":End of /LIST");
}
