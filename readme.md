# Multi-Threaded Client-Server Messaging System

## Overview

The YAMOTD (Yet Another Message of the Day) Server-Client Application is a versatile system designed to demonstrate the fundamentals of socket programming and client-server applications. This project involves the creation of a server and client application, both of which communicate over TCP sockets and adhere to the YAMOTD protocol.


## Key Features:

### Server Operations:
- Initialization with preloaded messages of the day.
- Handling incoming client requests.
- Message retrieval (MSGGET), message uploading (MSGSTORE), server shutdown (SHUTDOWN), and user authentication (LOGIN).
### Client Operations:
- Sending commands to the server: MSGGET, MSGSTORE, SHUTDOWN, LOGIN, LOGOUT, QUIT.
- Displaying server responses.

### YAMOTD Protocol:

- MSGGET: Retrieve a message of the day from the server.
- MSGSTORE: Upload a message to the server (authorized users only).
- SHUTDOWN: Terminate the server (root user privilege).
- LOGIN: Authenticate a user with a UserID and Password.
- LOGOUT: Log out from the server (restricts MSGSTORE, SHUTDOWN, WHO, and SEND).
- QUIT: Terminate the client-server connection.
- WHO: List all active users, including UserID and IP addresses.
- SEND: Send private messages to active users.

## Compilation Instructions

1. Create a directory for the programs.
2. Ensure that the directory includes the following files:
   - sclient.cpp
   - multiThreadServer.cpp
   - common.h
   - Makefile
3. Open a terminal and navigate to the project directory.
4. Execute the "make clean" command to clean the previous build artifacts.
5. Run the "make" command to compile the programs. You should observe output similar to the following:

   ```
   g++ -g -c sclient.cpp
   g++ -o sclient sclient.o
   g++ -g -c multiThreadServer.cpp
   g++ -o multiThreadServer multiThreadServer.o
   ```

6. Apply the same compilation process to both the client and server programs.

## Running the Programs

1. Open two terminal windows.
2. In the first terminal window, execute the following command:

   ```
   ./multiThreadServer
   ```

   You should see the message: "The server is up, waiting for connections."

3. In the second terminal window, enter the following command to run the client, replacing "127.0.0.1" with the appropriate IP address if necessary:

   ```
   ./sclient 127.0.0.1
   ```

   Alternatively, you can retrieve your IP address using the "curl ifconfig.me" command and use it with the client.

4. To connect additional clients, you can open a third terminal window and execute the same command as in step 3.

## Supported Commands

The client supports the following commands:

1. `msgget`: Sends a request to the server to retrieve messages from a file named "messages.txt" in chronological order.

2. `shutdown`: Allows the root user on the client side to shut down the server. This operation is restricted to the root user and will display a server shutdown message.

3. `login username password`: Permits the user on the client side to log in to access the server. The `login` command should be followed by the desired username and password. If the credentials are authenticated, the user is logged in; otherwise, an error message will be displayed.

4. `logout`: Allows the logged-in user to log out from the server.

5. `quit`: Terminates the client's connection to the server. A message indicating that the client has been terminated will be displayed.

6. `msgstore`: Enables the user to upload a single message to the server. The server will save the message to the "messages.txt" file, with a limit of 20 messages.

7. `who`: Provides a list of currently logged-in users along with their respective IP addresses.

8. `send`: Allows a user to send a message to another specified logged-in user.

## Sample Output for Client 1

```
who
200 OK
 The list of active users:
 root   127.0.0.1
login david david01
200 OK
msgget
200 OK
   You always pass failure on the way to success
msgstore
200 OK
Enter a message of the day
Hello
200 OK
who
200 OK
 The list of active users:
 david  141.215.69.204
 root   127.0.0.1
send john
420 either the user does not exist or is not logged in
send root
200 OK
what's up
200 OK
200 OK you have a new message from root
root: how u
210 the server is about to shutdown...
```

## Sample Output for Client 2

```
login root root01
200 OK
msgget
200 OK
   You always pass failure on the way to success
msgstore
200 OK
Enter a message of the day
I like turtles
200 OK
msgget
200 OK
   No one is perfect, that is why pencils have erasers
msgget
200 OK
   It always seems impossible until it is done
msgget
200 OK
   If opportunity does not knock, build a door
msgget
200 OK
   The difference between ordinary and extraordinary is that little extra
msgget
200 OK
   I like turtles

200 OK you have a new message from david
david: what's up
send root
200 OK
nothing much
200 OK you have a new message from root
root: nothing much
200 OK
send david
200 OK
how u
200 OK
shutdown
210 the server is about to shutdown...
```

## Sample Output for Server

```
multiThreadServer: new connection from 127.0.0.1 socket 4
LOGIN ROOT ROOT01
multiThreadServer: new connection from 141.215.69.204 socket 5
WHO
LOGIN DAVID DAVID01
MSGGET
MSGGET
MSGSTORE
Hello
MSGSTORE
I like turtles
MSGGET
MSGGET
MSGGET
MSGGET
MSGGET
WHO
SEND JOHN
SEND ROOT
SEND ROOT
SEND DAVID
how u
SHUTDOWN
System has shutdown
```

## Contributors:

- [Sonjoy Kumar Paul](https://github.com/sonjoykp)
- [Amrit Minocha](https://github.com/amritminocha)
