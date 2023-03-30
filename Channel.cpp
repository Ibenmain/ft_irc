#include "User.hpp"
#include "Channel.hpp"
#include "server.hpp"

Channel::Channel(const string& name): _name(name) {}

Channel::~Channel() { }

const string& Channel::getName(void) const {
    return _name;
}

bool Channel::findOperatorIfExistByNick(string nick) {
    map<int, User *>::iterator it;
    it = _operators.begin();
    for (; it != _operators.end(); it++)
    {
        if (it->second->getNickname() == "@" + nick)
            return (true);
    }
    return (false);
}

void Channel::broadcast(Server *server, int ignoreFd) const {
    map<int, User *>::const_iterator it;
    const string msg = server->createReplyForm();

    for(it = _userList.begin(); it != _userList.end(); ++it) {
        if (it->first == ignoreFd) continue;

        it->second->addToReplyBuffer(server->createReplyForm());
    }
}

const vector<string> Channel::getUserList(void) const {
    vector<string> userList;

    for (map<int, User *>::const_iterator it = _userList.begin(); it != _userList.end(); ++it) {
        string nickname = "";
        nickname += it->second->getNickname();
        userList.push_back(nickname);
    }
    return userList;
}

// void    setUserList(const string nickname)
// {
//     //  _userList.insert();
// }

void Channel::addUser(int clientFd, User *user) {
    _userList.insert(make_pair(clientFd, user));
}

void Channel::addOperators(int clientFd, User *user) {
    _operators.insert(make_pair(clientFd, user));
}

int Channel::deleteUser(int clientFd) {
    map<int, User *>::iterator it;
    // string clientSource;

    it = _userList.find(clientFd);
    if (it == _userList.end()) return _userList.size();
    
    // clientSource = it->second->getSource();
    _userList.erase(clientFd);

    if (_userList.empty()) return 0;
    return _userList.size();
}

// void Channel::deleteUser(int clientFd) {
//     map<int, User *>::iterator it;

//     it = _userList.find(clientFd);
//     if (it != _userList.end())
//         _userList.erase(clientFd);
// }

void Channel::deleteOperator(int clientFd) {
    map<int, User *>::iterator it;

    it = _operators.find(clientFd);
    if (it != _operators.end())
        _operators.erase(clientFd);
}

User* Channel::findUserByFd(const int clientFd) {
    map<int, User *>::iterator it;

    it = _userList.find(clientFd);
    if (it == _userList.end()) return NULL;
    return it->second;
}

bool Channel::findUserIfExist(const int clientFd) {
    map<int, User *>::iterator it;

    it = _userList.find(clientFd);
    if (it != _userList.end())
        return (true);
    return (false);
    // if (it == _userList.end()) return NULL;
    // return it->second;
}

bool Channel::findOperatorIfExist(const int clientFd) {
    map<int, User *>::iterator it;

    it = _operators.find(clientFd);
    if (it != _operators.end())
        return (true);
    return (false);
    // if (it == _userList.end()) return NULL;
    // return it->second;
}

User* Channel::findUserByNick(const string& nickname) {
    map<int, User *>::iterator it;

    for(it = _userList.begin(); it != _userList.end(); ++it) {
        User *user = it->second;

        if (user->getNickname() == nickname) return user;
    }
    return NULL;
}

void    Channel::getAllUser(void)
{
    map<int, User *>::iterator it;

    it = _userList.begin();
    for (; it != _userList.end(); it++)
    {
        cout << "Users: " << it->second->getNickname() << endl;
    }
}

const string    Channel::getUser(int fd)
{
    map<int, User *>::iterator it;

    it = _userList.find(fd);
    return(it->second->getNickname());
}

void    Channel::getOperator(void)
{
    map<int, User *>::iterator it;

    it = _operators.begin();
    for (; it != _operators.end(); it++)
    {
        cout << "Operators: " << it->second->getNickname() << endl;
    }
}


bool Channel::getFindPass() const
{
    return (_findPass);
}

void Channel::setFindPass(bool pass)
{
    _findPass = pass;
}

string Channel::getPassword() const
{
    return (_password);
}