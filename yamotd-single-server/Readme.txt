Project - 1: A client-server socket application

Group 9: Sonjoy Kumar Paul and Amrit Minocha

Within the scope of this project, you'll find two primary components: 
the client and the server. They have been designed to seamlessly interact 
with each other. The server file should run before the client file. Additionally, when firing 
up the client, it demands the IP address of the server to be fed as a command-line input.

How to run:
1. Carve out a separate folder to house the project files.
2. Make certain that this fresh folder is equipped with these three critical files:
	Client.cpp
	Server.cpp
	common.h
	Makefile
3. Go to the directory in terminal and type in "make" command.
4. Start the server: Open a terminal and run "./server"
5. Start the client: Open another terminal and run "./client 127.0.0.1"


Commands: 
1. msgget
The client issues a command to the server, requesting the "message of the day." 
The server retrieves this from the messages.txt file, delivering messages in the 
sequence they're recorded.

Enter a command: msgget
	Response from Server: 200 OK
			 Message of the day

2. shutdown
This command enables the client-side user to turn off the server. 
Following this, both the client and server processes cease to operate. 
Note that exclusively ROOT users hold the privilege to shut down the server.

Enter a command: shutdown
    Response from Server: 200 OK

3. login username password
The client-side user can utilize the login command to gain access to the server. 
This command necessitates both a username and password. Successful credential 
verification permits entry, but if they're mismatched, an "incorrect username/password" 
alert is raised.

Enter a command: login john john01
    Response from Server: 200 Ok

4. logout
This command enables a currently logged-in user to sign out from the server. 
Should no user be signed in, a response indicating "no users logged in" will be given.

Enter a command: logout
    Response from Server: 200 OK

5. quit 
It will terminate the client connection from server. The server will still
remain awake and will accept other connections from the client.

Enter a command: quit
    Response from Server: 200 Ok

6. msgstore message
It will allow the user to upload one message to the server. The server will save
the message to the messages.txt file. The limit for messages in 20.

Enter a command: msgstore message
    Response from Server: 200 Ok
