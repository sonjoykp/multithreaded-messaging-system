#include "common.h"

#define MAX_PENDING 5

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
	string responseMessage;

	int s;
	int new_s;
	int iCurrentDisplayedMeesageIndex;
	int iTotalNumberofMessages;
	string messages[20];
	bool isUserloggedIn;
	bool isRootUserloggedIn;
	bool isServerAlive;
	ofstream ofile;

	bool handleShutdown();
	void handleQuit();
	void handleMsgGet();
	void handleLogout();
	void handleLogin();
	void handleMsgStore();
};

Server::Server() : isUserloggedIn(false), isRootUserloggedIn(false), isServerAlive(true), iTotalNumberofMessages(0), iCurrentDisplayedMeesageIndex(0)
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
		if (!line.empty())
		{
			messages[cnt] = line;
			++cnt;
			++iTotalNumberofMessages;
		}
	}

	ifile.close();
}

bool Server::handleShutdown()
{
	if (isRootUserloggedIn)
	{
		isServerAlive = false;
		responseMessage = "Response from Server: 200 OK\n";
		send(new_s, responseMessage.c_str(), responseMessage.size() + 1, 0);
		return true;
	}
	else
	{
		responseMessage = "Response from Server: 402 User not allowed\n";
		return false;
	}
}

void Server::handleQuit()
{
	responseMessage = "Response from Server: 200 OK\n";
	send(new_s, responseMessage.c_str(), responseMessage.size() + 1, 0);
}

void Server::handleMsgGet()
{
	responseMessage = "Response from Server: 200 OK\n\t\t ";
	responseMessage += messages[iCurrentDisplayedMeesageIndex];
	responseMessage += "\n";
	iCurrentDisplayedMeesageIndex++;
	if (iCurrentDisplayedMeesageIndex == iTotalNumberofMessages)
		iCurrentDisplayedMeesageIndex = 0;
}

void Server::handleLogout()
{
	responseMessage = "Response from Server: No users logged in\n";
	if (isRootUserloggedIn)
	{
		responseMessage = "Response from Server: 200 OK\n";
		isRootUserloggedIn = false;
	}
	if (isUserloggedIn)
	{
		responseMessage = "Response from Server: 200 OK\n";
		isUserloggedIn = false;
	}
	iCurrentDisplayedMeesageIndex++;
}

void Server::handleLogin()
{
	string loginAcc1 = "login root root01";
	string loginAcc2 = "login john john01";
	string loginAcc3 = "login david david01";
	string loginAcc4 = "login mary mary01";

	if (isSameNoCase(recievedCommand, loginAcc2) ||
		isSameNoCase(recievedCommand, loginAcc3) ||
		isSameNoCase(recievedCommand, loginAcc4))
	{
		responseMessage = "Response from Server: 200 OK\n";
		isUserloggedIn = true;
		isRootUserloggedIn = false;
	}
	else if (isSameNoCase(recievedCommand, loginAcc1))
	{
		responseMessage = "Response from Server: 200 OK\n";
		isRootUserloggedIn = true;
		isUserloggedIn = false;
	}
	else
	{
		responseMessage = "Response from Server: 410 Wrong UserID or Password\n";
	}
}

void Server::handleMsgStore()
{
	if (isUserloggedIn || isRootUserloggedIn)
	{
		if (iTotalNumberofMessages < 20)
		{
			string messageToWrite = removePrefixNoCase(recievedCommand, msgstore_command);
			messageToWrite = removePrefixNoCase(messageToWrite, " ");

			if (!messageToWrite.empty())
			{
				messages[iTotalNumberofMessages] = messageToWrite;
				++iTotalNumberofMessages;

				// Check if the file is empty
				ofile.seekp(0, std::ios::end);
				if (ofile.tellp() != 0)
				{
					// If the file is not empty, add a newline before writing
					ofile << std::endl;
				}
				ofile << messageToWrite << endl;
			}

			responseMessage = "Response from Server: 200 OK\n";
		}
		else
		{
			responseMessage = "Response from Server: 402 No more space max limit exceed\n";
		}
	}
	else
	{
		responseMessage = "Response from Server: 401 You are not currently logged in, login first\n";
	}
}

void Server::run()
{
	while (isServerAlive)
	{
		if ((new_s = accept(s, (struct sockaddr *)&sin, &addrlen)) < 0)
		{
			perror("accept");
			exit(1);
		}
		cout << "new connection from " << inet_ntoa(sin.sin_addr) << endl;

		int ReadbufferLen = 0;
		while ((ReadbufferLen = recv(new_s, buf, sizeof(buf), 0)) && isServerAlive)
		{

			recievedCommand = string(buf, ReadbufferLen - 1);
			cout << recievedCommand << endl;

			// Shutdown Command
			// Description: It allows the user to shutdown the user
			if (isSameNoCase(recievedCommand, shutdown_command))
			{
				if (handleShutdown())
					break;
			}
			// Quit Command
			// Description: It allows the user to exit out of the server
			if (isSameNoCase(recievedCommand, quit_command))
			{
				handleQuit();
				break;
			}
			// MSGGET Command
			if (isSameNoCase(recievedCommand, msgget_command))
			{
				handleMsgGet();
			}
			// Logout Method
			// It will allow the user to logout.
			if (isSameNoCase(recievedCommand, logout_command))
			{
				handleLogout();
			}

			// Login Method
			// Allow the user to login
			if (startsWithNoCase(recievedCommand, login_command))
			{
				handleLogin();
			}

			// MSGStore
			// Once the user is logged in, it will allow them to send one message to the server
			if (startsWithNoCase(recievedCommand, msgstore_command))
			{
				handleMsgStore();
			}

			send(new_s, responseMessage.c_str(), responseMessage.size() + 1, 0);
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