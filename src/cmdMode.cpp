#include "Server.hpp"

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

void	Server::genericSendMode(int fd, std::string target, char mode, std::string arg, char sign){
	bool flag = false;
	if (sign == '+')
		flag = this->channels[target]->addModes(mode);
	else if (sign == '-')
		flag = this->channels[target]->removeModes(mode);
	if (flag == true || mode == 'l' || mode == 'k' || mode == 'o'){
		sendCode(fd, "324", clients[fd]->get_nick(), target + " :" + mode + " " + arg);
		_ToAll(channels[target], fd, "MODE " + target + " " + mode + " " + arg + "\r\n");
	}
}


//MODE <#canal> <+modos> [parâmetros]: Para definir modos em canais
void Server::handleMode(int fd, std::istringstream &command){
	std::string target, mode, arg;
	command >> target >> mode >> arg;
	if (checkMode(fd, target, mode, arg) == -1)
		return ;
	int user_fd = channels[target]->getByName(arg);

	if (mode[0] == '+'){
		for (size_t i = mode.size() - 1; i > 0; i--){
			if (mode[i] == 'i' && arg.empty()){ // Invite only
				channels[target]->setInviteChannel(true);
				genericSendMode(fd, target, mode[i], arg, '+');
			}
			else if (mode[i] == 'o' ){ // Give operator
				if (arg.empty()){
					sendCode(fd, "696", clients[fd]->get_nick(), target + " +o " + arg + " :You must specify a parameter for the operator mode. Syntax: <nickname>");
					continue;
				}
				channels[target]->addOperator(getClient(user_fd));
				genericSendMode(fd, target, mode[i], arg, '+');
			}
			else if (mode[i] == 'k'){ // Set key
				if(arg.empty()){
					sendCode(fd, "696", clients[fd]->get_nick(), target + " +k " + arg + " :You must specify a parameter for the key mode. Syntax: <key>");
					continue;
				}
				channels[target]->setKey(arg);
				genericSendMode(fd, target, mode[i], arg, '+');
			}
			else if (mode[i] == 'l' ){ // Set limit
				if(arg.empty()){
					sendCode(fd, "696", clients[fd]->get_nick(), target + " +l " + arg + " :You must specify a parameter for the limit mode. Syntax: <limit>");
					continue;
				}
			    std::stringstream ss(arg);
			    double limit;
			    ss >> limit;
				if (limit < 1 || limit > MAX_CLIENTS){
					sendCode(fd, "696", clients[fd]->get_nick(), target + " l " + arg + " :Invalid limit mode parameter. Syntax: <limit>");
				}
				else{
			    	channels[target]->setLimit(limit);
					genericSendMode(fd, target, mode[i], arg, '+');
				}
			}

		}
	}
	else if (mode[0] == '-'){
		for (size_t i = mode.size() - 1; i > 0; i--){
			if (mode[i] == 'i' && arg.empty()){ // Remove Invite only
				channels[target]->setInviteChannel(false);
				genericSendMode(fd, target, mode[i], arg, '-');
			}
			else if (mode[i] == 'o' ){ // Remove operator
				if (arg.empty()){
					sendCode(fd, "696", clients[fd]->get_nick(), target + " -o " + arg + " :You must specify a parameter for the operator mode. Syntax: <nickname>");
					continue;
				}
				channels[target]->removeOper(arg);
				genericSendMode(fd, target, mode[i], arg, '-');
			}
			else if (mode[i] == 'k' ){ // Remove key
				if (arg.empty()){
					sendCode(fd, "696", clients[fd]->get_nick(), target + " -k " + arg + " :You must specify a parameter for the key mode. Syntax: <key>");
					continue;
				}
				channels[target]->setKey("");
				genericSendMode(fd, target, mode[i], arg, '-');
			}
			else if (mode[i] == 'l' && arg.empty()){ // Remove limit
				channels[target]->setLimit(MAX_CLIENTS);
				genericSendMode(fd, target, mode[i], arg, '-');
			}
		}
	}

	else
		sendCode(fd, "472", clients[fd]->get_nick(), target + " :is not a recognised channel mode");
}

//No spotchat
// pelo botao
//<< MODE #meucanal +l 5
//<< MODE #meucanal
//>> @time=2024-10-05T07:46:05.940Z :a!~andre@SpotChat-2ro.k9p.92.185.IP MODE #meucanal +l :5
//>> @time=2024-10-05T07:46:05.978Z :lamia.uk.SpotChat.org 324 a #meucanal +l :5
//>> @time=2024-10-05T07:46:05.978Z :lamia.uk.SpotChat.org 329 a #meucanal :1728111326 - é o tempo que o canal foi criado

// pelo comando
//<< MODE #meucanal +l 8
//>> @time=2024-10-05T07:46:17.680Z :a!~andre@SpotChat-2ro.k9p.92.185.IP MODE #meucanal +l :8
