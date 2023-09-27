#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SERVER_PORT 9808
#define MAX_LINE 256

using namespace std;

class Client
{
public:
	Client(const char *serverIP);
	~Client();

	void run();

private:
	struct sockaddr_in sin;
	char buf[MAX_LINE];
	char fbuf[MAX_LINE];
	char rbuf[MAX_LINE];
	string quit = "quit";
	string msgget = "msgget";
	string login = "login";
	string logout = "logout";
	string shutdown = "shutdown";
	string msgstore = "msgstore";
	int len;
	int s;

	// void lowercaseInput();
	// void handleMsgGet();
	// void handleQuit();
	// void handleShutdown();
	// void handleLogout();
	// void handleLogin();
	// void handleMsgStore();
};

Client::Client(const char *serverIP)
{
	/* active open */
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}

	/* build address data structure */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(serverIP);
	sin.sin_port = htons(SERVER_PORT);

	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		perror("connect");
		close(s);
		exit(1);
	}

	cout << "Welcome to YAMOTD Project-1 CIS 527 Client Side" << endl;
}

Client::~Client()
{
	close(s);
}

// void Client::lowercaseInput() {
//     for (int i = 0; i < MAX_LINE; i++) {
//         buf[i] = tolower(buf[i]);
//     }
// }

// void Client::handleMsgGet() {
//     send(s, buf, strlen(buf) + 1, 0);
//     recv(s, rbuf, sizeof(rbuf), 0);
//     cout << rbuf << endl;
// }

// void Client::handleQuit() {
//     send(s, buf, strlen(buf) + 1, 0);
//     recv(s, rbuf, sizeof(rbuf), 0);
//     cout << rbuf << endl;
// }

// void Client::handleShutdown() {
//     send(s, buf, strlen(buf) + 1, 0);
//     recv(s, rbuf, sizeof(rbuf), 0);
//     cout << rbuf << endl;
//     // If server replies this, the client will close too
//     string temp = "Response from Server: 200 OK\n";
//     if (strcmp(rbuf, temp.c_str()) == 0) {
//         exit(0);
//     }
// }

// void Client::handleLogout() {
//     send(s, buf, strlen(buf) + 1, 0);
//     recv(s, rbuf, sizeof(rbuf), 0);
//     cout << rbuf << endl;
// }

// void Client::handleLogin() {
//     send(s, buf, strlen(buf) + 1, 0);
//     recv(s, rbuf, sizeof(rbuf), 0);
//     cout << rbuf << endl;
// }

// void Client::handleMsgStore() {
//     send(s, buf, strlen(buf) + 1, 0);
//     recv(s, rbuf, sizeof(rbuf), 0);
//     cout << rbuf << endl;
// }

void Client::run()
{
	while (true)
	{
		// User interaction at client side
		cout << "Enter a command: ";
		fgets(buf, sizeof(buf), stdin);

		// Lowercases all input to unify commands
		for (int i = 0; i < MAX_LINE; i++)
		{
			buf[i] = tolower(buf[i]);
		}

		buf[MAX_LINE - 1] = '\0';
		len = strlen(buf) + 1;

		// MSGGET

		if (strcmp(buf, msgget.c_str()) == 10) // The ASCII value of the newline character (\n) is 10 in decimal notation and 0x0A in hexadecimal notation.
		{
			send(s, buf, len, 0);
			recv(s, rbuf, sizeof(rbuf), 0);
			cout << rbuf << endl;
		}

		// QUIT

		if (strcmp(buf, quit.c_str()) == 10)
		{
			send(s, buf, len, 0);
			recv(s, rbuf, sizeof(rbuf), 0);
			cout << rbuf << endl;
			close(s);
			break;
		}

		// SHUTDOWN

		if (strcmp(buf, shutdown.c_str()) == 10)
		{
			send(s, buf, len, 0);
			recv(s, rbuf, sizeof(rbuf), 0);
			cout << rbuf << endl;
			// If server replies this, the client will close too
			string temp = "Response from Server: 200 OK\n";
			if (strcmp(rbuf, temp.c_str()) == 0)
			{
				break;
			}
		}

		// LOGOUT

		if (strcmp(buf, logout.c_str()) == 10)
		{
			send(s, buf, len, 0);
			recv(s, rbuf, sizeof(rbuf), 0);
			cout << rbuf << endl;
		}

		// LOGIN user pass

		strcpy(fbuf, buf);
		if (strcmp(fbuf, login.c_str()) == 32)
		{
			strncpy(buf, fbuf, 6);

			if (strcmp(buf, login.c_str()) == 32)
			{

				send(s, fbuf, len, 0);
				recv(s, rbuf, sizeof(rbuf), 0);
				cout << rbuf << endl;
			}
		}

		// MSGSTORE message

		strcpy(fbuf, buf);
		if (strcmp(fbuf, msgstore.c_str()) == 32) // The ASCII value of the space character (' ') is 32 in decimal notation.
		{
			strncpy(buf, fbuf, 9);

			if (strcmp(buf, msgstore.c_str()) == 32)
			{
				send(s, buf, len, 0);
				recv(s, rbuf, sizeof(rbuf), 0);
				cout << rbuf << endl;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cerr << "Usage: client <Server IP Address>" << endl;
		exit(1);
	}

	Client client(argv[1]);
	client.run();

	cout << "End of Project-1 Client side" << endl;

	return 0;
}