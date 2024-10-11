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
	if (clients[fd] && (clients[fd]->getUser().empty() 
		|| clients[fd]->getPass().empty() 
		|| clients[fd]->getNick().empty())){
		sendCode(fd, "461", "", "Not enough parameters"); //ERR_NEEDMOREPARAMS
		return;
	}
	if (clients[fd]->getAuth() == true){
		sendCode(fd, "462", clients[fd]->getNick(), ": You may not reauth"); // ERR_ALREADYREGISTRED
		return;
	}
	if (strcmp(clients[fd]->getPass().c_str(), _pass.c_str()) == 0){
		sendCode(fd, "001", clients[fd]->getNick(), ":Welcome to the 42Porto IRC Network " + clients[fd]->getMask());
		sendCode(fd, "002", clients[fd]->getNick(), ":Your host is " + clients[fd]->getHost() + ", running version 1.0");
		sendCode(fd, "003", clients[fd]->getNick(), ":This server was created " + serverTimestamp());
		sendCode(fd, "004", clients[fd]->getNick(), clients[fd]->getHost() + " InspIRCd-3 BDHIORSTWcdghikorswxz ACIJKMNOPQRSTYabceghiklmnopqrstvz :IJYabeghkloqv"); //! Explicacao no arquivo explain.txt
		
		sendCode(fd, "005", clients[fd]->getNick(), "CHANMODES=Ibeg,k,Jl,ACKMNOPQRSTiprstz :are supported by this server");


		//sendCode(fd, "005", clients[fd]->get_nick(), ":This server was created " + clients[fd]->getHost());
		sendCode(fd, "371", clients[fd]->getNick(), ":User is Authenticated");
		sendCode(fd, "375", clients[fd]->getNick(), ":" + clients[fd]->getHost() + " Message of the day");
		sendCode(fd, "372", clients[fd]->getNick(), ":    ▟██▛╗██▛███   ");
		sendCode(fd, "372", clients[fd]->getNick(), ":  ▟██▛╔═╝█▛ ▟█▛╗");
		sendCode(fd, "372", clients[fd]->getNick(), ":▟██▛╔═╝   ╚▟▛╔═╝                       ▝▜▉▛▘█▀▜▉▐▛▀▜");
		sendCode(fd, "372", clients[fd]->getNick(), ":████████╗▟█▛ ▟█╗ █▀█ █▀█ █▀█ ▀█▀ █▀█    ▐▉▌ █▄▟▀▐▌");
		sendCode(fd, "372", clients[fd]->getNick(), ": ╚═══███║███▟██║ █▀▀ █▄█ █▀▄  █  █▄█ ▁▁▗▟▉▙▖▉ ▐▙▐▙▄▟");
		sendCode(fd, "372", clients[fd]->getNick(), ":     ███║ ╚════╝ ");
		sendCode(fd, "372", clients[fd]->getNick(), ":      ╚═╝			By: bmonteir; jodos-sa; mamaral-");
		sendCode(fd, "376", clients[fd]->getNick(), ":End of message of the day.");
		sendCode(fd, "374", clients[fd]->getNick(), ":Now you can join channels and chat with other users");
		sendCode(fd, "396", clients[fd]->getNick(), clients[fd]->getHost() + " :is now your displayed host");
		clients[fd]->setAuth(true);
		return ;
	}
	else
		sendCode(fd, "372", clients[fd]->getNick(), ": User is not Authenticated");
}

//! PASS should come before NICK and USER, maybe we should create a function to force the others commands to come only after PASS.
void Server::handlePass(int fd, std::istringstream &command){
	std::string pass;
	command >> pass;
	pass = extract_value(pass);
	if (pass.empty())
		sendCode(fd, "461", "", "Not enough parameters"); //ERR_NEEDMOREPARAMS
	else if (clients[fd]->getAuth() == true)
		sendCode(fd, "462", clients[fd]->getNick(), ": You may not reregister"); // ERR_ALREADYREGISTRED
	else if (_pass != pass)
		sendCode(fd, "464", clients[fd]->getNick(), ": Password incorrect");
	else {
		clients[fd]->setPass(pass);
		sendCode(fd, "338", clients[fd]->getNick(), ": Password accepted");
	}
}

void Server::handleNick(int fd, std::istringstream &command){
	std::string nick;
	command >> nick;
	
	if (nick.empty()){
		sendCode(fd, "431", "", "No nickname given"); // ERR_NONICKNAMEGIVEN
		clients[fd]->setNick("\0");
		return;
	}
	std::map<int, Client *>::iterator it;
	for(it = clients.begin(); it != clients.end(); it++){
		if (it->second->getNick() == nick){
			sendCode(fd, "433", nick, ":Nickname is already in use");
			this->clients[fd]->setNick(nick);
			// clients[fd]->set_flagNick(true);
			return;
		}
	}
	if (this->clients[fd]->getNick().empty())
		this->clients[fd]->setNick(nick);
	else{
		std::string changeNick = ":" + this->clients[fd]->getNick() + " NICK " + nick + "\r\n";
		std::string nickChangeMsg = this->clients[fd]->getMask() + "NICK :" + nick + "\r\n";
		print_client(fd, changeNick);
		this->clients[fd]->setNick(nick);
		_ToAll(fd, nickChangeMsg);
	}
	clients[fd]->setMask(":" + clients[fd]->getNick() + "!" + clients[fd]->getUser() + "@" + clients[fd]->getHost() + " ");
}

void Server::handleUser(int fd, std::istringstream &command){
	std::string username, hostname, servername, realname;
	command >> username >> hostname >> servername; 
	std::getline(command, realname); 
	if (username.empty()){
		sendCode(fd, "461", "", "Not enough parameters"); // ERR_NEEDMOREPARAMS
		return;
	}
	if (clients[fd]->getUser().empty()){
		this->clients[fd]->setUser(username);
		this->clients[fd]->setRealname(realname.substr(2, realname.size() - 3));
		if (clients[fd]->getNick() != "\0")
			clients[fd]->setMask(":" + clients[fd]->getNick() + "!" + clients[fd]->getUser() + "@" + clients[fd]->getHost() + " ");
	}
	else{
		sendCode(fd, "462", clients[fd]->getNick(), ": You may not reregister"); // ERR_ALREADYREGISTRED
	}
}

void Server::handleJoin(int fd, std::istringstream &command){
	std::string channelName, key, line;
	command >> line >> key;
	std::stringstream ss(line);

	if (clients[fd]->getAuth() == false){
		print_client(fd, "Need to Auth the user\n");
		sendCode(fd, "451", clients[fd]->getNick(), ": You have not registered"); // ERR_NOTREGISTERED
		return ;
	}
	if (line.empty()){
		sendCode(fd, "461", "", "Not enough parameters"); // ERR_NEEDMOREPARAMS
		return ;
	}

	while (std::getline(ss, channelName, ',')){
		if (channelName[0] != '#'){
			sendCode(fd, "476", clients[fd]->getNick(), channelName + " :Invalid channel name"); // ERR_BADCHANMASK
			return ;
		}
		if (getChannel(channelName) == NULL)
			createChannel(channelName, fd);
		if ((this->channels[channelName]->getInviteChannel() == true 
		&& this->channels[channelName]->getInviteList().find(fd) == this->channels[channelName]->getInviteList().end())
		&& this->channels[channelName]->getOperators().find(fd) == this->channels[channelName]->getOperators().end()){
			sendCode(fd, "473", clients[fd]->getNick(), channelName + " :Cannot join channel (+i) invite only"); // ERR_INVITEONLYCHAN
			return ;
		}
		if (!this->channels[channelName]->getKey().empty() && this->channels[channelName]->getKey() != key){
			sendCode(fd, "475", clients[fd]->getNick(), channelName + " :Cannot join channel (+k) bad key"); // ERR_BADCHANNELKEY
			return ;
		}
		if (this->channels[channelName]->getUsers().size() >= this->channels[channelName]->getLimit() ){
			sendCode(fd, "471", clients[fd]->getNick(), channelName + " :Cannot join channel (+l) limit reached"); // ERR_CHANNELISFULL
			return ;
		}

		this->clients[fd]->addChannel(channelName, *this->channels[channelName]);
		
		this->channels[channelName]->addUser(getClient(fd));
		
		print_client(fd, clients[fd]->getMask() + "JOIN :" + channelName + "\r\n");

		// if (!this->channels[channelName]->getTopic().empty())
		sendCode(fd, "332", clients[fd]->getNick(), channelName + " " + this->channels[channelName]->getTopic());
		sendCode(fd, "353", clients[fd]->getNick() + " = " + channelName, this->channels[channelName]->listAllUsers());
		sendCode(fd, "366", clients[fd]->getNick(), channelName + " :End of /NAMES list");
		_ToAll(this->channels[channelName], fd, "JOIN :" + channelName + "\r\n");
	}
}

void Server::handlePrivmsg(int fd, std::istringstream &command){
	std::string target, message;
	command >> target;
	std::getline(command, message);
	if (target.empty() || message.empty()){
		sendCode(fd, "411", clients[fd]->getNick(), ": No recipient given (PRIVMSG)");
		return ;
	}
	if (target[0] == '#'){
		if (channels.find(target) == channels.end()){
			sendCode(fd, "401", clients[fd]->getNick(), target + " :No such nick/channel");
			return ;
		}
		if (channels[target]->getUsers().find(fd) == channels[target]->getUsers().end()){
			sendCode(fd, "404", clients[fd]->getNick(), target + " :Cannot send to channel");
			return ;
		}
		_ToAll(channels[target], fd, "PRIVMSG " + target + " " + message + "\n");
	}
	else{
		int receiver_fd = 0;
		std::map<int, Client *>::iterator it = this->clients.begin();
		while(it != this->clients.end())
		{
			if (it->second->getNick() == target){
					receiver_fd = it->second->get_client_fd();
				break;
			}
			it++;
		}
		// _sendall(receiver_fd, clients[fd]->getMask() + "PRIVMSG " + target + " " + message + "\n");
		if(receiver_fd)
			print_client(receiver_fd, clients[fd]->getMask() + "PRIVMSG " + target + " " + message + "\n");
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
	if (channel->getUsers().find(fd) == channel->getUsers().end()){
		print_client(fd, "You are not in this channel\n");
		return ;
	}
	print_client(fd, clients[fd]->getMask() + "PART " + channelName + "\r\n");
	_ToAll(channel, fd, "PART " + channelName + "\r\n");
	channel->removeUser(clients[fd]->getNick());
	channel->removeOper(clients[fd]->getNick());
	if (this->channels[channelName]->listAllUsers() == ":"){
		delete this->channels[channelName];
		this->channels.erase(channelName);
	}
	clients[fd]->removeChannel(channelName);
}

void Server::handleQuit(int fd, std::istringstream &command){
	std::string message;
	command >> message;
	std::string response = clients[fd]->getMask() + "QUIT :" + message + "\r\n";
	_ToAll(fd, response);
	for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); it++){
		if (it->second->getUsers().find(fd) != it->second->getUsers().end()){
			it->second->removeUser(clients[fd]->getNick());
			it->second->removeOper(clients[fd]->getNick());
			// if (it->second->listAllUsers() == ":"){
			// 	delete it->second;
			clients[fd]->removeChannel(it->first);
			// 	this->channels.erase(it);
			// }
	// 		else
			//clients[fd]->removeChannel(it->first);
		}
	}
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, clients[fd]->get_client_fd(), NULL) == -1) {
		std::cerr << "Error removing socket from epoll(quit): " << strerror(errno) << std::endl;
	}
	close(clients[fd]->get_client_fd());
	delete clients[fd];
	this->clients.erase(fd);
	this->_cur_online--;
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
	sendCode(fd, "PONG", clients[fd]->getHost(), server);
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
		if (it->second->getMask().find(mask) != std::string::npos){
			sendCode(fd, "352", clients[fd]->getNick(), mask + " " + it->second->getUser() + " " + it->second->getHost() + " " + it->second->getHost() + " " + it->second->getNick() + " H :0 " + it->second->getRealname());
		}
		it++;
	}
	sendCode(fd, "315", clients[fd]->getNick(), mask + " :End of /WHO list");
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
		if (it->second->getMask().find(mask) != std::string::npos){
			sendCode(fd, "311", clients[fd]->getNick(), mask + " " + it->second->getUser() + " " + it->second->getHost() + " * :" + it->second->getRealname());
			sendCode(fd, "312", clients[fd]->getNick(), mask + " " + it->second->getHost() + " :42Porto");
			sendCode(fd, "319", clients[fd]->getNick(), mask + " :");
		}
		it++;
	}
	sendCode(fd, "318", clients[fd]->getNick(), mask + " :End of /WHOIS list");
}

void Server::handleList(int fd){
	for (std::map<std::string, Channel *>::iterator it = channels.begin(); it != channels.end(); it++){
		std::stringstream ss;
		ss << it->second->getUsers().size();
		sendCode(fd, "322", clients[fd]->getNick(), it->first + " " + ss.str() + " :" + it->second->getTopic());
	}
	sendCode(fd, "323", clients[fd]->getNick(), ":End of /LIST");
}
