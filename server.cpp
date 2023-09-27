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

class Server
{
public:
	Server();
	~Server();
	void run();

private:
	struct sockaddr_in sin;
	socklen_t addrlen;
	char buf[MAX_LINE];
	char tempb[MAX_LINE];
	int len;
	int s;
	int new_s;
	int i = 0;
	int itotal = 0;
	string messages[20];
	string quit = "quit";
	string msgget = "msgget";
	bool islogin = false;
	bool isRlogin = false;
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
	ifstream ifile;
	ofstream ofile;

	void substring(char s[], char sub[], int p, int l);
	// void handleShutdown();
	// void handleQuit();
	// void handleMsgGet();
	// void handleLogout();
	// void handleLogin();
	// void handleMsgStore();
};

Server::Server() : islogin(false), isRlogin(false), shutdown(false), itotal(0)
{
	// File read and store messages from
	ifile.open("messages.txt");
	int j = 0;
	if (ifile.is_open())
	{
		getline(ifile, messages[j]);
		messages[j] += '\n';
		messages[j] += '\0';
		while (ifile)
		{
			++j;
			++itotal;
			getline(ifile, messages[j]);
			messages[j] += '\n';
			messages[j] += '\0';
		}
	}
	ifile.close();

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

void Server::substring(char s[], char sub[], int p, int l)
{
	int c = 0;
	while (c < l)
	{
		sub[c] = s[p + c - 1];
		c++;
	}
	sub[c] = '\0';
}

// void Server::handleShutdown() {
//     if (isRlogin) {
//         shutdown = true;
// 		buf = "Response from Server: 200 OK\n";
//         strcpy(buf, "Response from Server: 200 OK\n");
//     } else {
//         strcpy(buf, "Response from Server: 402 User not allowed\n");
//     }
// }

// void Server::handleQuit() {
//     strcpy(buf, "Response from Server: 200 OK\n");
// }

// void Server::handleMsgGet() {
//     buf = "Response from Server: 200 OK\n\t\t " + messages[0] + "\n";
//     strcpy(buf, temp.c_str());
// }

// void Server::handleLogout() {
//     buf = "Response from Server: No users logged in\n";
//     if (isRlogin) {
//         buf = "Response from Server: 200 OK\n";
//         isRlogin = false;
//     }
//     if (islogin) {
//         buf = "Response from Server: 200 OK\n";
//         islogin = false;
//     }
// }

// void Server::handleLogin() {
//     buf = "Response from Server: 200 OK\n";
//     islogin = true;

//     strcpy(buf, temp.c_str());
// }

// void Server::handleMsgStore() {
//     char tempb[MAX_LINE];
//     substring(buf, tempb, 10, strlen(buf) - 10);
//     if (islogin || isRlogin) {
//         if (itotal < 20) {
//             string temp(tempb);
//             messages[itotal] = temp;
//             ++itotal;
//             temp = '\n' + tempb;
//             ofile.write(temp.c_str(), strlen(tempb) + 1);
//             temp = "Response from Server: 200 OK\n";
//             strcpy(buf, temp.c_str());
//         } else {
//             strcpy(buf, "Response from Server: 402 No more space max limit exceed\n");
//         }
//     } else {
//         strcpy(buf, "Response from Server: 401 You are not currently logged in, login first\n");
//     }
// }

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

		while ((len = recv(new_s, buf, sizeof(buf), 0)) && !shutdown)
		{
			cout << buf;

			// Shutdown Command
			// Description: It allows the user to shutdown the user
			if (strcmp(buf, stdown.c_str()) == 10) // The ASCII value of the newline character (\n) is 10 in decimal notation and 0x0A in hexadecimal notation.
			{
				if (isRlogin)
				{
					shutdown = true;
					temp = "Response from Server: 200 OK\n";
					strcpy(buf, temp.c_str());
					send(new_s, buf, strlen(buf) + 1, 0);
					break;
				}
				else
				{
					temp = "Response from Server: 402 User not allowed\n";
					strcpy(buf, temp.c_str());
				}
			}
			// Quit Command
			// Description: It allows the user to exit out of the server
			if (strcmp(buf, quit.c_str()) == 10)
			{
				temp = "Response from Server: 200 OK\n";
				strcpy(buf, temp.c_str());
				send(new_s, buf, strlen(buf) + 1, 0);
				break;
			}
			// MSGGET Command
			if (strcmp(buf, msgget.c_str()) == 10)
			{
				temp = "Response from Server: 200 OK\n\t\t ";
				temp += messages[i];
				temp += "\n";
				strcpy(buf, temp.c_str());
				i++;
				if (i == itotal)
					i = 0;
			}
			// Logout Method
			// It will allow the user to logout.
			if (strcmp(buf, logout.c_str()) == 10)
			{
				temp = "Response from Server: No users logged in\n";
				if (isRlogin)
				{
					temp = "Response from Server: 200 OK\n";
					isRlogin = false;
				}
				if (islogin)
				{
					temp = "Response from Server: 200 OK\n";
					islogin = false;
				}
				strcpy(buf, temp.c_str());
				i++;
			}

			// cout << "*" << strcmp(buf, login.c_str()) << endl;
			// Login Method
			// Allow the user to login
			if (strcmp(buf, loginAcc2.c_str()) == 10 ||
				strcmp(buf, loginAcc3.c_str()) == 10 ||
				strcmp(buf, loginAcc4.c_str()) == 10)
			{
				temp = "Response from Server: 200 OK\n";
				islogin = true;
				strcpy(buf, temp.c_str());
			}
			else if (strcmp(buf, loginAcc1.c_str()) == 10)
			{
				temp = "Response from Server: 200 OK\n";
				isRlogin = true;
				strcpy(buf, temp.c_str());
			}
			else if (strcmp(buf, login.c_str()) == 32) // The ASCII value of the space character (' ') is 32 in decimal notation.
			{
				temp = "Response from Server: 410 Wrong UserID or Password\n";
				strcpy(buf, temp.c_str());
			}
			// MSGStore
			// Once the user is logged in, it will allow them to send one message to the server

			if (strcmp(buf, msgstore.c_str()) == 32)
			{
				substring(buf, tempb, 10, strlen(buf) - 10);
				if (islogin || isRlogin)
				{
					if (itotal < 20)
					{
						temp = tempb;
						messages[itotal] = temp;
						++itotal;
						temp = '\n';
						temp += tempb;
						ofile.write(temp.c_str(), strlen(tempb) + 1);
						temp = "Response from Server: 200 OK\n";
						strcpy(buf, temp.c_str());
					}
					else
					{
						temp = "Response from Server: 402 No more space max limit exceed\n";
						strcpy(buf, temp.c_str());
					}
				}
				else
				{
					temp = "Response from Server: 401 You are not currently logged in, login first\n";
					strcpy(buf, temp.c_str());
				}
			}

			send(new_s, buf, strlen(buf) + 1, 0);
		}
		ofile.close();
		close(new_s);
	}

	cout << "End of Project-1 Server side" << endl;
}

int main()
{
	Server server;
	server.run();
	return 0;
}