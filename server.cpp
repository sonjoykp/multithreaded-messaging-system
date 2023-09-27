#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace std;

#define SERVER_PORT 9808
#define MAX_PENDING 5
#define MAX_LINE 256

bool startsWithNoCase(const std::string &str, const std::string &prefix)
{
	if (str.length() < prefix.length())
	{
		return false; // The string is shorter than the prefix, so it can't start with it.
	}

	for (size_t i = 0; i < prefix.length(); ++i)
	{
		if (std::tolower(str[i]) != std::tolower(prefix[i]))
		{
			return false; // Characters don't match in a case-insensitive comparison.
		}
	}

	return true;
}

bool isSameNoCase(const std::string &str1, const std::string &str2)
{
	if (str1.length() != str2.length())
	{
		return false; // If the lengths are different, the strings cannot be the same.
	}

	for (size_t i = 0; i < str1.length(); ++i)
	{
		if (std::tolower(str1[i]) != std::tolower(str2[i]))
		{
			return false; // Characters don't match in a case-insensitive comparison.
		}
	}

	return true;
}

std::string removePrefixNoCase(const std::string &str, const std::string &prefix)
{
	// Check if the string starts with the prefix (case-insensitive)
	size_t str_len = str.length();
	size_t prefix_len = prefix.length();

	if (str_len >= prefix_len)
	{
		bool match = true;
		for (size_t i = 0; i < prefix_len; ++i)
		{
			if (std::tolower(str[i]) != std::tolower(prefix[i]))
			{
				match = false;
				break;
			}
		}

		if (match)
		{
			// Return the string with the prefix removed
			return str.substr(prefix_len);
		}
	}

	// If the prefix is not found or couldn't be removed, return the original string
	return str;
}

class Server
{
public:
	Server();
	~Server();
	void readAndStoreInitMessagesFromFile();
	void run();

private:
	struct sockaddr_in sin;
	socklen_t addrlen;
	char buf[MAX_LINE];
	string recievedCommand;
	string response;

	int s;
	int new_s;
	int iCurrentDisplayedMeesageIndex = 0;
	int iTotalNumberofMessages = 0;
	string messages[20];
	bool isUserloggedIn = false;
	bool isRootUserloggedIn = false;
	string quit = "quit";
	string msgget = "msgget";
	string login = "login";
	string logout = "logout";
	string stdown = "shutdown";
	string msgstore = "msgstore";
	bool shutdown = false;
	// Credentials for the Login Command
	string loginAcc1 = "login root root01";
	string loginAcc2 = "login john john01";
	string loginAcc3 = "login david david01";
	string loginAcc4 = "login mary mary01";
	string temp;
	ofstream ofile;

	bool handleShutdown();
	void handleQuit();
	void handleMsgGet();
	void handleLogout();
	void handleLogin();
	void handleMsgStore();
};

Server::Server() : isUserloggedIn(false), isRootUserloggedIn(false), shutdown(false), iTotalNumberofMessages(0)
{

	// File write and store messages to

	ofile.open("messages.txt", ios::out | ios::app);

	/* build address data structure */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(SERVER_PORT);

	/* setup passive open */
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}

	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0)
	{
		perror("bind");
		exit(1);
	}

	listen(s, MAX_PENDING);

	addrlen = sizeof(sin);

	cout << "Welcome to YAMOTD Project-1 CIS 527 Server Side" << endl;
	cout << "The server is up, waiting for connection" << endl;
}

Server::~Server()
{
	close(s);
}

void Server::readAndStoreInitMessagesFromFile()
{
	// File read and store messages from
	ifstream ifile;
	ifile.open("messages.txt");
	int cnt = 0;

	string line;
	while (getline(ifile, line))
	{
		messages[cnt] = line;
		++cnt;
		++iTotalNumberofMessages;
	}

	ifile.close();
}

bool Server::handleShutdown()
{
	if (isRootUserloggedIn)
	{
		shutdown = true;
		response = "Response from Server: 200 OK\n";
		send(new_s, response.c_str(), response.size() + 1, 0);
		return true;
	}
	else
	{
		response = "Response from Server: 402 User not allowed\n";
		return false;
	}
}

void Server::handleQuit()
{
	response = "Response from Server: 200 OK\n";
	send(new_s, response.c_str(), response.size() + 1, 0);
}

void Server::handleMsgGet()
{
	response = "Response from Server: 200 OK\n\t\t ";
	response += messages[iCurrentDisplayedMeesageIndex];
	response += "\n";
	iCurrentDisplayedMeesageIndex++;
	if (iCurrentDisplayedMeesageIndex == iTotalNumberofMessages)
		iCurrentDisplayedMeesageIndex = 0;
}

void Server::handleLogout()
{
	response = "Response from Server: No users logged in\n";
	if (isRootUserloggedIn)
	{
		response = "Response from Server: 200 OK\n";
		isRootUserloggedIn = false;
	}
	if (isUserloggedIn)
	{
		response = "Response from Server: 200 OK\n";
		isUserloggedIn = false;
	}
	iCurrentDisplayedMeesageIndex++;
}

void Server::handleLogin()
{
	if (isSameNoCase(recievedCommand, loginAcc2) ||
		isSameNoCase(recievedCommand, loginAcc3) ||
		isSameNoCase(recievedCommand, loginAcc4))
	{
		response = "Response from Server: 200 OK\n";
		isUserloggedIn = true;
		isRootUserloggedIn = false;
	}
	else if (isSameNoCase(recievedCommand, loginAcc1))
	{
		response = "Response from Server: 200 OK\n";
		isRootUserloggedIn = true;
		isUserloggedIn = false;
	}
	else
	{
		response = "Response from Server: 410 Wrong UserID or Password\n";
	}
}

void Server::handleMsgStore()
{
	if (isUserloggedIn || isRootUserloggedIn)
	{
		if (iTotalNumberofMessages < 20)
		{
			string messageToWrite = removePrefixNoCase(recievedCommand, msgstore);
			messageToWrite = removePrefixNoCase(messageToWrite, " ");

			if (recievedCommand.size() > 0)
			{
				messages[iTotalNumberofMessages] = messageToWrite;
				++iTotalNumberofMessages;
				ofile << messageToWrite << endl;
			}

			response = "Response from Server: 200 OK\n";
		}
		else
		{
			response = "Response from Server: 402 No more space max limit exceed\n";
		}
	}
	else
	{
		response = "Response from Server: 401 You are not currently logged in, login first\n";
	}
}

void Server::run()
{
	while (!shutdown)
	{
		if ((new_s = accept(s, (struct sockaddr *)&sin, &addrlen)) < 0)
		{
			perror("accept");
			exit(1);
		}
		cout << "new connection from " << inet_ntoa(sin.sin_addr) << endl;

		int ReadbufferLen = 0;
		while ((ReadbufferLen = recv(new_s, buf, sizeof(buf), 0)) && !shutdown)
		{

			recievedCommand = string(buf, ReadbufferLen - 1);
			cout << buf << endl;

			// Shutdown Command
			// Description: It allows the user to shutdown the user
			if (isSameNoCase(recievedCommand, stdown))
			{
				if (handleShutdown())
					break;
			}
			// Quit Command
			// Description: It allows the user to exit out of the server
			if (isSameNoCase(recievedCommand, quit))
			{
				handleQuit();
				break;
			}
			// MSGGET Command
			if (isSameNoCase(recievedCommand, msgget))
			{
				handleMsgGet();
			}
			// Logout Method
			// It will allow the user to logout.
			if (isSameNoCase(recievedCommand, logout))
			{
				handleLogout();
			}

			// Login Method
			// Allow the user to login
			if (startsWithNoCase(recievedCommand, login))
			{
				handleLogin();
			}

			// MSGStore
			// Once the user is logged in, it will allow them to send one message to the server
			if (startsWithNoCase(recievedCommand, msgstore))
			{
				handleMsgStore();
			}

			send(new_s, response.c_str(), response.size() + 1, 0);
		}
		ofile.close();
		close(new_s);
	}

	cout << "End of Project-1 Server side" << endl;
}

int main()
{
	Server server;
	server.readAndStoreInitMessagesFromFile();
	server.run();
	return 0;
}