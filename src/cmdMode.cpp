#include "Server.hpp"

bool isStringDigit(const std::string& str) {
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        if (!isdigit(*it)) {
            return false;
        }
    }
    return true;
}

int	Server::checkMode(int fd, std::string &target, std::string &mode, std::string &arg){
	if (!target.empty() && target[0] != '#' && mode.empty() ){
		sendCode(fd, "401", clients[fd]->get_nick(), target + " :No such nick/channel");
		return -1;
	}
	if (target.empty()){ // /MODE <+modos>
		sendCode(fd, "461", "", "Not enough parameters in MODE");
		return -1;
	}
	if (!target.empty() && target[0] == '#' && mode.empty() && arg.empty()){ //MODE <#canal>  
		if (channels.find(target) == channels.end()){
			sendCode(fd, "403", clients[fd]->get_nick(), target + " :No such channel");
			return -1;
		}
		sendCode(fd, "324", clients[fd]->get_nick(), target + " " + channels[target]->getModes());
		std::stringstream ss;
		ss << channels[target]->getCreationTime();
		sendCode(fd, "329", clients[fd]->get_nick(), target + " " + ss.str());
		return -1;
	}
	if (!target.empty() && target[0] == '#'){
		if (channels.find(target) == channels.end()){
			sendCode(fd, "401", clients[fd]->get_nick(), target + " :No such nick/channel");
			return -1;
		}
		if (channels[target]->getUsers().find(fd) == channels[target]->getUsers().end()){
			sendCode(fd, "404", clients[fd]->get_nick(), target + " :Cannot send to channel");
			return -1;
		}
		if (channels[target]->getOperators().find(fd) == channels[target]->getOperators().end()){
			sendCode(fd, "482", clients[fd]->get_nick(), target + " :You're not channel operator");
			return -1;
		}
	}
	return 0;
}

//MODE <#canal> <+modos> [parâmetros]: Para definir modos em canais
void Server::handleMode(int fd, std::istringstream &command){
	std::string target, mode, arg;
	command >> target >> mode >> arg;
	if (checkMode(fd, target, mode, arg) == -1)
		return ;
	int user_fd = channels[target]->getByName(arg);
	if (!arg.empty()){
		if (mode == "+o" )
			channels[target]->addOperator(getClient(user_fd));
		else if (mode == "-o" )
			channels[target]->removeOper(arg);
		else if (mode == "+i")
			channels[target]->setInviteChannel(true);
		else if (mode == "-i")
			channels[target]->setInviteChannel(false);
		else if (mode == "+k")
			channels[target]->setKey(arg);
		else if (mode == "-k")
			channels[target]->setKey("");
		else if (mode == "+l"){
			std::stringstream ss(arg);
			int limit;
			ss >> limit;
			channels[target]->setLimit(limit);
		}
		else if (mode == "-l")
			channels[target]->setLimit(10000);
		else if (mode == "+t") ;
		else if (mode == "-t") ;
		else if (mode == "+n") ;
		else if (mode == "-n") ;
		else {
			sendCode(fd, "472", clients[fd]->get_nick(), target + " :is unknown mode char to me");
			return;
		}
		// sendCode(fd, "324", clients[fd]->get_nick(), target + " " + mode + " :" + arg);
		print_client(fd, clients[fd]->get_mask() + "MODE " + target + " " + mode + " :" + arg + "\r\n");
		_ToAll(channels[target], fd, "MODE " + target + " " + mode + " :" + arg + "\r\n");
	}
	else{
		if (mode == "+i")
			channels[target]->setInviteChannel(true);
		else if (mode == "-i")
			channels[target]->setInviteChannel(false);
		else if (mode == "-k")
			channels[target]->setKey("");
		else if (mode == "-l")
			channels[target]->setLimit(10000);
		else if (mode == "+t") ;
		else if (mode == "-t") ;
		else if (mode == "+n") ;
		else if (mode == "-n") ;
		else {
			sendCode(fd, "472", clients[fd]->get_nick(), target + " :is unknown mode char to me");
			return;
		}
		// sendCode(fd, "324", clients[fd]->get_nick(), target + " :" + mode);
		print_client(fd, clients[fd]->get_mask() + "MODE " + target + " :" + mode + "\r\n");
		_ToAll(channels[target], fd, "MODE " + target + " :" + mode + "\r\n");
	}
	if (mode[0] == '+')
		this->channels[target]->addModes(mode.substr(1));
	else if (mode[0] == '-')
		this->channels[target]->removeModes(mode.substr(1));
}

	// if (mode == "+i" && arg.empty()){ // Invite only
	// 	channels[target]->setInviteChannel(true);
	// 	sendCode(fd, "324", clients[fd]->get_nick(), target + " :+i ");
	// 	_ToAll(channels[target], fd, "MODE " + target + " +i " + "\r\n");
	// 	this->channels[target]->addModes("i");
	// }
	// else if (mode == "-i" && arg.empty()){ // Remove Invite only
	// 	channels[target]->setInviteChannel(false);
	// 	sendCode(fd, "324", clients[fd]->get_nick(), target + " :-i ");
	// 	_ToAll(channels[target], fd, "MODE " + target + " -i " + "\r\n");
	// 	this->channels[target]->removeModes("i");
	// }
	
 	// else if (mode == "+o" && !arg.empty()){ // Give operator
	// 	channels[target]->addOperator(getClient(user_fd));
	// 	sendCode(fd, "324", clients[fd]->get_nick(), target + " +o " + arg);
	// 	_ToAll(channels[target], fd, "MODE " + target + " +o " + arg + "\r\n");
	// 	this->channels[target]->addModes("o");
	// }
	// else if (mode == "-o" && !arg.empty()){ // Remove operator
	// 	channels[target]->removeOper(arg);
	// 	sendCode(fd, "324", clients[fd]->get_nick(), target + " -o " + arg);
	// 	_ToAll(channels[target], fd, "MODE " + target + " -o " + arg + "\r\n");
	// 	this->channels[target]->removeModes("o");
	// }

	// else if (mode == "+k" && !arg.empty()){ // Set key
	// 	channels[target]->setKey(arg);
	// 	sendCode(fd, "324", clients[fd]->get_nick(), target + " +k :" + arg);
	// 	_ToAll(channels[target], fd, "MODE " + target + " +k :" + arg + "\r\n");
	// 	this->channels[target]->addModes("k");
	// }
	// else if (mode == "-k" && arg.empty()){ // Remove key
	// 	channels[target]->setKey("");
	// 	sendCode(fd, "324", clients[fd]->get_nick(), target + " :-k ");
	// 	_ToAll(channels[target], fd, "MODE " + target + " :-k " + "\r\n");
	// 	this->channels[target]->removeModes("k");
	// }

	// else if (mode == "+l" && !arg.empty()){ // Set limit
	//     std::stringstream ss(arg);
	//     int limit;
	//     ss >> limit;
	//     channels[target]->setLimit(limit);
	// 	sendCode(fd, "324", clients[fd]->get_nick(), target + " +l :" + arg);
	//     _ToAll(channels[target], fd, "MODE " + target + " +l :" + arg + "\r\n");
	// 	this->channels[target]->addModes("l");
	// }
	// else if (mode == "-l" && arg.empty()){ // Remove limit
	// 	channels[target]->setLimit(10000);
	// 	sendCode(fd, "324", clients[fd]->get_nick(), target + " -l :");
	// 	_ToAll(channels[target], fd, "MODE " + target + " -l :" + "\r\n");
	// 	this->channels[target]->removeModes("l");
	// }
	// else if (mode == "+t" && arg.empty()){ // Set topic
	// 	channels[target]->addModes("t");
	// 	sendCode(fd, "324", clients[fd]->get_nick(), target + " :+t ");
	// 	_ToAll(channels[target], fd, "MODE " + target + " :+t " + "\r\n");
	// }
	// else if (mode == "-t" && arg.empty()){ // Remove topic
	// 	channels[target]->removeModes("t");
	// 	sendCode(fd, "324", clients[fd]->get_nick(), target + " :-t ");
	// 	_ToAll(channels[target], fd, "MODE " + target + " :-t " + "\r\n");
	// }
	// else if (mode == "+n" && arg.empty()){ // Set no external messages
	// 	channels[target]->addModes("n");
	// 	sendCode(fd, "324", clients[fd]->get_nick(), target + " :+n ");
	// 	_ToAll(channels[target], fd, "MODE " + target + " :+n " + "\r\n");
	// }
	// else if (mode == "-n" && arg.empty()){ // Remove no external messages
	// 	channels[target]->removeModes("n");
	// 	sendCode(fd, "324", clients[fd]->get_nick(), target + " :-n ");
	// 	_ToAll(channels[target], fd, "MODE " + target + " :-n " + "\r\n");
	// }
	// else{
	// 	sendCode(fd, "472", clients[fd]->get_nick(), target + " :is unknown mode char to me");
	// }
