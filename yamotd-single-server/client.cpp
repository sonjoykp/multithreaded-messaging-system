#include "common.h"


class Client
{
public:
	Client(const char *serverIP);
	~Client();

	void run();

private:
	struct sockaddr_in sin;
	char rbuf[MAX_LINE];

	string inputCommand;
	string messageToSend;


	int s;

	void handleMsgGet();
	void handleQuit();
	bool handleShutdown();
	void handleLogout();
	void handleLogin();
	void handleMsgStore();
	void sendRecievePrint();
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

void Client::sendRecievePrint()
{
	send(s, inputCommand.c_str(), inputCommand.size() + 1, 0);
	recv(s, rbuf, sizeof(rbuf), 0);
	cout << rbuf << endl;
}

bool Client::handleShutdown()
{
	sendRecievePrint();
	// If server replies this, the client will close too
	string temp = "Response from Server: 200 OK\n";
	if (strcmp(rbuf, temp.c_str()) == 0)
	{
		return true;
	}

	return false;
}

void Client::run()
{
	while (true)
	{
		// User interaction at client side
		cout << "Enter a command: ";
		getline(cin, inputCommand);
		//cout << inputCommand << endl;

		// MSGGET, LOGOUT, LOGIN, MSGSTORE commands
		if (isSameNoCase(inputCommand, msgget_command) || 
		    isSameNoCase(inputCommand, logout_command) || 
			startsWithNoCase(inputCommand, login_command) || 
			startsWithNoCase(inputCommand, msgstore_command))
		{
			sendRecievePrint();
		}

		// QUIT
		if (isSameNoCase(inputCommand, quit_command))
		{
			sendRecievePrint();
			close(s);
			break;
		}

		// SHUTDOWN
		if (isSameNoCase(inputCommand, shutdown_command))
		{
			if (handleShutdown())
			{
				break;
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