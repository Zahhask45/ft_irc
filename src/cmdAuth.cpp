#include "Server.hpp"

static std::string serverTimestamp(){
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
	return buf;
}

std::string checkAuth(const std::string arg){
	if (arg.empty())
		return "error";
	return "check";
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
	else if(nick == bot->get_name()){
			sendCode(fd, "433", nick, ":Cannot use the bot nickname");
	}
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
