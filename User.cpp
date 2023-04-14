/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sghajdao <sghajdao@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/06 01:10:27 by ibenmain          #+#    #+#             */
/*   Updated: 2023/04/13 16:49:11 by sghajdao         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include "User.hpp"
#include "server.hpp"


User::User(int fd, const string& host) : _sd(fd), _host(host), _isQuiting(false) {

}

User::~User() {
    close(_sd);
}

User::User(void){}

int User::getFd(void) const {
    return _sd;
}

const string& User::getHost(void) const {
    return _host;
}

string User::getPassword(void) {
    return _password;
}

const string User::getNickname(void) const {
    if (_nickname.empty()) return "*";

    return _nickname;
}

const string User::getSource(void) const {
    string source = getNickname();
    if (source.empty()) source = "*";
    source = source + "@" + _host;

    return source;
}

const string& User::getUsername(void) const {
    return _username;
}

const string& User::getHostname(void) const {
    return _hostname;
}

const string& User::getServername(void) const {
    return _servername;
}

const string& User::getRealname(void) const {
    return _realname;
}

const string& User::getCmdBuffer(void) const {
    return _cmdBuffer;
}

const string& User::getReplyBuffer(void) const {
    return _replyBuffer;
}

bool User::getIsQuiting(void) const {
    return _isQuiting;
}

bool User::getIsUser(void) const {
    return _user;
}

bool User::getIsPass(void) const {
    return _pass;
}

bool User::getIsNick(void) const {
    return _nick;
}

const string& User::getReason(void) const {
    return _reason;
}

const string& User::getChannelLeave(void) const {
    return _channel_leave;
}

void User::setFd(int fd) {
    _sd = fd;
}
void User::setPassword(const string& pwd) {
    _password = pwd;
}

void User::setNickname(const string& nickname) {
    _nickname = nickname;
}

void User::setUsername(const string& username) {
    _username = username;
}

void User::setHostname(const string& hostname) {
    _hostname = hostname;
}

void User::setServername(const string& servername) {
    _servername = servername;
}

void User::setRealname(const string& realname) {
    _realname = realname;
}

void User::setCmdBuffer(const string& str) {
    _cmdBuffer = str;
}

void User::clearCmdBuffer(void) {
    _cmdBuffer.clear();
}

void User::setReplyBuffer(const string& str) {
    _replyBuffer = str;
}

void User::clearReplyBuffer(void) {
    _replyBuffer.clear();
}

void User::addToCmdBuffer(const string& str) {
    _cmdBuffer.append(str);
}

void User::addToReplyBuffer(const string& str) {
    _replyBuffer.append(str);
}

void User::setIsQuiting(void) {
    _isQuiting = true;
}

void User::setIsUser(void) {
    _user = true;
}

void User::setIsPass(void) {
    _pass = true;
}

void User::setIsNick(void) {
    _nick = true;
}

int  User::getRegistred(void) const
{
    return (registred);
}

void User::setRegistred(void)
{
    registred = true;
}

void User::setReason(const string& reason) {
    _reason = reason;
}

void User::setChannelLeave(const string& channel) {
    _channel_leave = channel;
}

User& User::operator=(const User& user)
{
	this->_nickname = user._nickname;
	return(*this);
}

User::User(const User& user)
{
    *this = user;
}

void User::addChannelUser(string channel){
	_channelOfUser.push_back(channel);
}

void User::deleteChannelUser(string channel){
    vector<string>::iterator it;

    it = _channelOfUser.begin();
    if (it->find(channel))
        _channelOfUser.erase(it);
}

void User::addChannelOperator(string channel){
	_channelOfOperatore.push_back(channel);
}

void User::deleteChannelOperator(string channel){
vector<string>::iterator it;

    it = _channelOfOperatore.begin();
    if (it->find(channel))
        _channelOfOperatore.erase(it);
}

bool User::SearchChannelOperator(string channel){

    for (size_t i = 0; i < _channelOfOperatore.size(); i++)
    {
        if (_channelOfOperatore[i] == channel)
            return (true);
    }
    return (false);
}

bool User::SearchChannelUser(string channel){

    for (size_t i = 0; i < _channelOfUser.size(); i++)
    {
        if (_channelOfUser[i] == channel)
            return (true);
    }
    return (false);
}

string User::ft_hostname()
{
    char hostname[50];

    if (gethostname(hostname, sizeof(hostname)) == -1)
        cout << "Error :hostname\n";
    struct hostent *host = gethostbyname(hostname);
    char * szLocalIP;
    szLocalIP = inet_ntoa (*(struct in_addr*)*host->h_addr_list);
    return (szLocalIP);

}

vector<string> User::getUser()
{
    return (_channelOfUser);

}
vector<string> User::getChannelList()
{
    return (_channelOfUser);
}
