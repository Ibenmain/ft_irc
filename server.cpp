/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ibenmain <ibenmain@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/28 05:47:13 by mlalouli          #+#    #+#             */
/*   Updated: 2023/04/12 17:25:31 by ibenmain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "messagerror.hpp"
#include "server.hpp"
#include "User.hpp"

Server::Server(int port, string password): _sd(-1), _kq(-1), _port(port), _password(password) {
	struct sockaddr_in serverAddr;
	if ((_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		shutDown("socket() error");

	memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(_port);
	if (fcntl(_sd, F_SETFL, O_NONBLOCK) < 0)
		shutDown("fcntl() error");
	updateEvents(_sd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	if (::bind(_sd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
        shutDown("bind() error");

	if (listen(_sd, 5) == -1)
        shutDown("listen() error");
}

const vector<string>& Server::getParams(void) const {
    return _params;
}

const string& Server::getCommand(void) const {
    return _command;
}

void Server::updateEvents(int socket, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata) {
	struct kevent event;
	/* initialize a struct kevent object, which is used to represent an event */
	EV_SET(&event, socket, filter, flags, fflags, data, udata);
	eventList.push_back(event);
}

void Server::createNewClientSocket(void) {
	int clientSocket;
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);
	char hostStr[INET_ADDRSTRLEN];
	User *user;

	memset(hostStr, 0, sizeof(hostStr));
	memset(&clientAddr, 0, sizeof(clientAddr));
	if ((clientSocket = accept(_sd, (struct sockaddr *)&clientAddr, &addrLen)) == -1) {
		cerr << "aceept() failed! Check errno : " << errno << endl;
		errno = 0;
		return ;
	}
	if (_allUser.size() >= 30) {
		cout << "Server reached max number of user" << endl;
		close(clientSocket);
		return ;
	}
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);
	// if (getnameinfo((struct sockaddr *)&clientAddr, sizeof(clientAddr), hostStr, INET_ADDRSTRLEN, NULL, NI_MAXSERV, NI_NUMERICSERV) < 0) {
	// 	cout << "Error while getting hostname on new client..." << endl;
	// 	return ;
	// }
	cout << hostStr << endl;
	inet_ntop(AF_INET, &clientAddr.sin_addr, hostStr, INET_ADDRSTRLEN);
	cout << "accept new client: " << clientSocket << " / Host : " << hostStr << endl;
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);

	updateEvents(clientSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
	updateEvents(clientSocket, EVFILT_WRITE, EV_ADD, 0, 0, NULL);

	user = new User(clientSocket, hostStr);
	_allUser.insert(make_pair(clientSocket, user));
}

void Server::run() {
	int numOfEvents;

	if ((_kq = kqueue()) == -1)
        throw(runtime_error("kqueue() error"));
	cout << "listening..." << endl;
	while (1) {
        numOfEvents = kevent(_kq, &eventList[0], eventList.size(), _waitingEvents, 8, NULL);
        if (numOfEvents == -1)
            shutDown("kevent() error");
        eventList.clear();
        for (int i = 0; i < numOfEvents; ++i)
            handleEvent(_waitingEvents[i]);
    }
}

void Server::recvClientData(const struct kevent& event) {
	char buf[501];
	map<int, User *>::iterator it = _allUser.find(event.ident);
	User* targetUser = it->second;
	int recvBytes;

	if (it == _allUser.end()) return ;
	recvBytes = recv(event.ident, buf, 500, 0);
	if (recvBytes <= 0) {
		if (recvBytes == -1 && errno == EAGAIN) {
			errno = 0;
			return;
		}
		cerr << "client recv error!" << endl;
		_allUser.erase(event.ident);
		cout << "client disconnected: " << event.ident << '\n';
	} else {
		buf[recvBytes] = '\0';
		targetUser->addToCmdBuffer(buf);
		handleCmd(targetUser, event);
	}
	targetUser->clearCmdBuffer();
}

bool Server::getUesrNickname(string nickname)
{
    map<int, User *>::iterator it;
    it = _allUser.begin();

    for (; it != _allUser.end(); it++)
    {
    	if (it->second->getNickname() == nickname)
    		return (1);
    }
    return (0);
};

void	Server::sendMessage(const struct kevent& event, std::string msg)
{
	int sendBytes;
	sendBytes = send(event.ident, msg.c_str(), msg.size(), 0);
	if (sendBytes <= 0) {
		if (sendBytes == -1 && errno == EAGAIN) {
			errno = 0;
			return;
		}
		cerr << "client send error!" << endl;
		_allUser.erase(event.ident);
		cout << "client disconnected: " << event.ident << '\n';
	}
}

void	Server::checkPassword(std::vector<string> tab, User *user, const struct kevent& event)
{
	if (tab.size() < 1)
	 	sendMessage_error(user->getNickname(), event, ERR_NEEDMOREPARAMS, 461);
	else if(tab.size() > 1)
	 	sendMessage_error(user->getNickname(), event, ERROR, 404);
	else
	{
		if (tab[0].compare(getpassword()) == 0)
		{
	 		user->setPassword(tab[0]);
			user->setIsPass();
		}
		else
	 		sendMessage_error(user->getNickname(), event, ERR_PASSWDMISMATCH, 464);
	}
}

int		Server::checkNickExist(vector<string> tab, User* user, const struct kevent& event)
{
	(void)user;
	(void)event;
	for (map<int, User *>::iterator it = _allUser.begin(); it != _allUser.end(); it++)
	{
		if (tab[0].compare(it->second->getNickname()) == 0)
			return (1);
	}
	return (0);
}

void	Server::checkUser(std::vector<string> tab, User* user, const struct kevent& event)
{
	if (tab.size() < 4)
	 	sendMessage_error(user->getNickname(), event, ERR_NEEDMOREPARAMS, 461);
	// else if(tab.size() > 4)
	//  	sendMessage_error(user->getNickname(), event, ERROR, 404);
	else
	{
		// if (checkUserExist(tab, user, event))
		// 	sendMessage_error(user->getNickname(), event, ERR_ALREADYREGISTERED, 464);
		// else
		// {
	 		user->setUsername(tab[0]);
	 		user->setHostname(tab[1]);
	 		user->setServername(tab[2]);
	 		user->setRealname(tab[3]);
			user->setIsUser();
		// }
	}
}

void	Server::checkNick(std::vector<string> tab, User* user, const struct kevent& event)
{
	if (tab.size() < 1)
	 	sendMessage_error(user->getNickname(), event, ERR_NEEDMOREPARAMS, 461);
	else if(tab.size() > 1)
	 	sendMessage_error(user->getNickname(), event, ERROR, 404);
	else
	{
		if (checkNickExist(tab, user, event))
			sendMessage_error(user->getNickname(), event, ERR_ALREADYREGISTERED, 464);
		else
		{
	 		user->setNickname(tab[0]);
			user->setIsNick();
		}
	}
}

void	Server::__parssingCommand(User* user, const struct kevent& event)
{
	// int x = 0;
	if (!user->getRegistred())
	{
		if ((_command != "PASS" && _command != "pass") && (_command != "USER" && _command != "user") && (_command != "NICK" && _command != "nick"))
			sendMessage_error(user->getNickname(), event, ERR_REGISTERED, 000);
		else if (_command == "PASS" || _command != "pass"|| _command == "USER" || _command != "user" || _command == "NICK" || _command != "nick")
		{
			if ((_command.compare("PASS") == 0 || _command.compare("pass") == 0) && !user->getIsPass())
				Server::checkPassword(_params, user, event);
			else if((_command.compare("USER") == 0 || _command.compare("user") == 0) && !user->getIsUser())
				Server::checkUser(_params, user, event);
			else if(_command.compare("NICK") == 0 || _command.compare("nick") == 0)
				Server::checkNick(_params, user, event);
		}
		if (user->getIsNick() && user->getIsUser() && user->getIsPass())
			Server::authentication(_params, user, event);
	}
	else if (_command.compare("PRIVMSG") == 0 || _command.compare("privmsg") == 0)
		cmdPrivmsg(user, event);
	else if (_command.compare("JOIN") == 0 || _command.compare("join") == 0)
		cmdJoin(user, event, _params);
	else if (_command.compare("PART") == 0 || _command.compare("part") == 0)
		cmdPart(user, event, _params);
	else if (_command.compare("INVITE") == 0 || _command.compare("invite") == 0)
		cmdInvite(user, event, _params);
	else if (_command.compare("MODE") == 0 || _command.compare("mode") == 0)
		cmdMode(user, event, _params);
	else if (_command.compare("NOTICE") == 0 || _command.compare("notice") == 0)
		cmdNotice(user, event);
	else if (_command.compare("KICK") == 0 || _command.compare("kick") == 0)
		cmdKick(user, event);
	else if (_command.compare("TOPIC") == 0 || _command.compare("topic") == 0)
		cmdTopic(user, event, _params);
	else if (_command.compare("QUIT") == 0 || _command.compare("quit") == 0)
		cmdQuit(user, event, _params);
	else if (_command.compare("JOKE") == 0 || _command.compare("joke") == 0)
		boot(event);
	else
		sendMessage_error(_command, event, " :Command not found", 912);
	user->clearCmdBuffer();
}

void	Server::sendMessageWelcom(string buffer, User* user, const struct kevent& event)
{
	(void)user;
	int	   sendBytes;
	sendBytes = send(event.ident, buffer.c_str(), buffer.size(), 0);
	if (sendBytes <= 0) {
		cerr << "client send error!" << endl;
		_allUser.erase(event.ident);
		cout << "client disconnected: " << event.ident << '\n';
		return;
	}
}

void	Server::authentication(std::vector<string> tab, User* user, const struct kevent& event)
{
	(void)tab;
	// time_t now = time(0);
	// char *date_time = ctime(&now);
	string hostname = user->ft_hostname();
	std::string buffer = ":" + hostname + " 001 " +  user->getNickname() +  " :Welcome to the Internet Relay Network " + user->getNickname() + "!~" + user->getNickname() + "@" + hostname +"\r\n";
    buffer += ":" + hostname + " 002 " +  user->getNickname() + " :Your host is " + hostname + ", running version leet-irc 1.0.0\r\n";
    buffer += ":" + hostname + " 003 " +  user->getNickname() + " " + hostname + " leet-irc 1.0.0 aioOrsw aovimntklbeI\r\n";
    buffer += ":" + hostname + " 004 " +  user->getNickname() + " :This server has been started Wed Oct 12 2022\r\n";
	buffer += ":" + hostname + " 372 " + user->getNickname() +  " :         ┬ ┬┌─┐┬  ┌─┐┌─┐┌┬┐┌─┐  ┌┬┐┌─┐  ┬┬─┐┌─┐  ┌─┐┌─┐┬─┐┬  ┬┌─┐┬─┐\r\n";
	buffer += ":" + hostname + " 372 " + user->getNickname() +  " :         │││├┤ │  │  │ ││││├┤    │ │ │  │├┬┘│    └─┐├┤ ├┬┘└┐┌┘├┤ ├┬┘\r\n";
	buffer += ":" + hostname + " 372 " + user->getNickname() +  " :         └┴┘└─┘┴─┘└─┘└─┘┴ ┴└─┘   ┴ └─┘  ┴┴└─└─┘  └─┘└─┘┴└─ └┘ └─┘┴└─\r\n";
	buffer += ":" + hostname + " 372 " + user->getNickname() +  " :                        Please enjoy your stay!\r\n";
	buffer += ":" + hostname + " 372 " + user->getNickname() +  " :you can use bot command (BOT) to get quote of the day!\r\n";
	buffer += ":" + hostname + " 376 " + user->getNickname() +  " :Made by lalalalala\r\n";
	sendMessageWelcom(buffer, user, event);
	user->setRegistred();
}

void Server::sendDataToClient(const struct kevent& event) {
	map<int, User *>::iterator it = _allUser.find(event.ident);
	User* targetUser = it->second;
	// int sendBytes;

	if (it == _allUser.end()) return ;
	if (targetUser->getReplyBuffer().empty()) return;

	// sendBytes = send(event.ident, targetUser->getReplyBuffer().c_str(), targetUser->getReplyBuffer().length(), 0);
	// if (sendBytes == -1) {
	// 	cerr << "client send error!" << endl; 
	// 	return ;
	// } else {
	// 	targetUser->setReplyBuffer(targetUser->getReplyBuffer().substr(sendBytes));
	// 	cout << targetUser->getReplyBuffer() << endl;
	// 	if (targetUser->getIsQuiting() && targetUser->getReplyBuffer().empty()) _allUser.erase(event.ident);
	// }
	sendMessage(event, targetUser->getReplyBuffer());
	targetUser->setReplyBuffer("");
}

void Server::handleEvent(const struct kevent& event) {
	if (event.flags & EV_ERROR) {
		if (event.ident == (const uintptr_t)_sd)
			throw(runtime_error("server socket error"));
		else {
			// User *targetUser = _allUser[event.ident];
			cerr << "client socket error" << endl;
		}
	} else if (event.filter == EVFILT_READ) {
		if (event.ident == (const uintptr_t)_sd)
			createNewClientSocket();
		else
			recvClientData(event);}
	else if (event.filter == EVFILT_WRITE)
		sendDataToClient(event);
}

void Server::shutDown(const string& msg) {
	if (_sd != -1)
		close(_sd);
	if (_kq != -1)
		close(_kq);
	for (map<int, User *>::iterator it = _allUser.begin(); it != _allUser.end(); it++) {
		delete it->second;
	}
	cerr << msg << endl;
	exit(EXIT_FAILURE);
}

std::string Server::getpassword()
{
	return(_password);
}
