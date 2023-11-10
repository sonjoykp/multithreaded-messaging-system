#include "common.h"

using namespace std;

fd_set master; // master file descriptor list
int listener;  // listening socket descriptor
int fdmax;
struct sockaddr_in remoteaddr;

unordered_map<int, string> SocketVSAddress;
unordered_map<int, string> SocketVsUserName;
int iTotalNumberofMessages;
string message_list[20]; // stores the messages from txt file in array

std::string extractLoginUsername(const std::string &inputString)
{
    std::istringstream iss(inputString);
    std::string login, username, password;

    if (!(iss >> login >> username >> password) || login != "login")
    {
        // Either "login" not found or incorrect format
        return "";
    }

    return username;
}

void readAndStoreInitMessagesFromFile()
{
    // File read and store messages from
    ifstream ifile;
    ifile.open("messages.txt");
    int cnt = 0;

    string line;
    while (getline(ifile, line))
    {
        if (!line.empty() && iTotalNumberofMessages < 20)
        {
            message_list[cnt] = line;
            ++cnt;
            ++iTotalNumberofMessages;
        }
    }

    ifile.close();
}

int getLoggedinUserSocketNumber(string userName)
{ // if the loggedin user found return socket number othewise return INT_MAX
    for (auto &user : SocketVsUserName)
    {
        if (isSameNoCase(user.second, userName))
            return user.first;
    }
    return INT_MAX;
}

// the child thread
void *ChildThread(void *newfd)
{
    char buf[MAX_LINE];

    int nbytes;
    int i, j;
    int childSocket = (long)newfd;

    int iCurrentDisplayedMeesageIndex = 0;

    // string wh = "WHO";
    string sendJohnCommand = "SEND JOHN";
    string sendMaryCommand = "SEND MARY";
    string sendDavidCommand = "SEND DAVID";
    string sendRootCommand = "SEND ROOT";

    string currentLoggedinUserName;

    bool isUserLoggedIn = false; // conditions
    bool serverShutdown = false;
    bool isRootUserLoggedIn = false;

    string loginRootCommand = "login root root01";
    string loginJohnCommand = "login john john01";
    string loginDavidCommand = "login david david01";
    string loginMaryCommand = "login mary mary01";

    while (!serverShutdown)
    {
        while (1)
        {
            // handle data from a client
            if ((nbytes = recv(childSocket, buf, sizeof(buf), 0)) <= 0)
            {
                // got error or connection closed by client
                if (nbytes == 0)
                {
                    // connection closed
                    cout << "multiThreadServer: socket " << childSocket << " hung up" << endl;
                }
                else
                {
                    perror("recv");
                }
                close(childSocket);           // bye!
                FD_CLR(childSocket, &master); // remove from master set
                pthread_exit(0);
            }
            else
            {
                string recievedCommand = string(buf, nbytes - 1);
                cout << recievedCommand << endl;
                if (isSameNoCase(recievedCommand, who_command)) // WHO Command Begins
                {
                    string responseMessage = "200 OK\n The list of active users: \n";
                    for (const auto &pair : SocketVsUserName)
                    {
                        responseMessage += " ";
                        responseMessage += pair.second;
                        responseMessage += "\t";
                        responseMessage += SocketVSAddress[pair.first];
                        responseMessage += "\n";
                    }
                    send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                    break;
                }                                                       // WHO Command Ends
                else if (isSameNoCase(recievedCommand, msgget_command)) // MSGGET Command Begins
                {
                    string responseMessage = "200 OK\n   ";
                    responseMessage += message_list[iCurrentDisplayedMeesageIndex];
                    responseMessage += "\n";

                    iCurrentDisplayedMeesageIndex++;
                    if (iCurrentDisplayedMeesageIndex == iTotalNumberofMessages)
                        iCurrentDisplayedMeesageIndex = 0;

                    send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                    break;

                }                                                             // MSGGET Command Ends
                else if (startsWithNoCase(recievedCommand, msgstore_command)) // MSGSTORE Command Begins
                {
                    string responseMessage;
                    if (isUserLoggedIn)
                    {
                        if (iTotalNumberofMessages < 20)
                        {
                            responseMessage = server_sucess_message;
                            // responseMessage += "Enter a message of the day:\n"; // added message for user friendliness
                            // sends 200 OK, then waits to  receive the user's message
                            send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);

                            char getBuffer[MAX_LINE];
                            int byteCnt = recv(childSocket, getBuffer, sizeof(getBuffer), 0);
                            string newMessageofTheDay = string(getBuffer, byteCnt - 1); // converts the message into a string and stores it in a variable
                            if (!newMessageofTheDay.empty())
                            {
                                message_list[iTotalNumberofMessages] = newMessageofTheDay;
                                ++iTotalNumberofMessages;

                                ofstream ofile;
                                ofile.open("messages.txt", ios::app);
                                // Check if the file is empty
                                ofile.seekp(0, std::ios::end);
                                if (ofile.tellp() != 0)
                                {
                                    // If the file is not empty, add a newline before writing
                                    ofile << std::endl;
                                }
                                ofile << newMessageofTheDay;
                                ofile.close();
                                responseMessage = server_sucess_message;
                            }
                        }
                        else
                        {
                            responseMessage = "402 No more space max limit exceed\n";
                        }
                    }
                    else // error condition if user is not logged in
                    {
                        responseMessage = "401 You are not currently logged in, login first\n";
                    }

                    send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                    break;

                } // MSGSTORE Command Ends
                else if (startsWithNoCase(recievedCommand, login_command))
                {
                    string responseMessage;
                    if (isSameNoCase(recievedCommand, loginDavidCommand) ||
                        isSameNoCase(recievedCommand, loginJohnCommand) ||
                        isSameNoCase(recievedCommand, loginMaryCommand) ||
                        isSameNoCase(recievedCommand, loginRootCommand)) // LOGIN Command Begins
                    {
                        string userName = extractLoginUsername(recievedCommand);
                        SocketVsUserName[childSocket] = userName;
                        currentLoggedinUserName = userName;

                        isUserLoggedIn = true; // for users that are not the root
                        if (isSameNoCase(userName, root_username))
                            isRootUserLoggedIn = true;

                        responseMessage = server_sucess_message;
                    }
                    else // error case for any other users
                    {
                        responseMessage = "410 Wrong UserID or Password\n";
                    }

                    send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                    break;

                } // LOGIN Command Ends
                else if (startsWithNoCase(recievedCommand, send_command))
                {
                    string responseMessage;
                    if (!isUserLoggedIn)
                    {
                        responseMessage = "401 You are not currently logged in, login first\n";
                        send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                        break;
                    }
                    else if (isSameNoCase(recievedCommand, sendJohnCommand) ||
                             isSameNoCase(recievedCommand, sendDavidCommand) ||
                             isSameNoCase(recievedCommand, sendMaryCommand) ||
                             isSameNoCase(recievedCommand, sendRootCommand))
                    { // SEND Command for john begins (same logic for all cases)
                        string TargetUserName = removePrefixNoCase(recievedCommand, send_command + " ");
                        int targetUserSocketNumber = getLoggedinUserSocketNumber(TargetUserName);
                        cout << "Target Socket number: " << targetUserSocketNumber << endl;

                        if (targetUserSocketNumber == INT_MAX) // error condition check
                        {
                            responseMessage.clear();
                            responseMessage = "420 either the user does not exist or is not logged in\n";
                            send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                            break;
                        }
                        else
                        {
                            responseMessage.clear();
                            responseMessage = server_sucess_message;
                            send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                            char sendMessageBuffer[MAX_LINE];
                            int bytecnt = recv(childSocket, sendMessageBuffer, sizeof(sendMessageBuffer), 0);
                            if (bytecnt < 0)
                            {
                                responseMessage.clear();
                                responseMessage = "s: Could not read message to send.";
                                send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                                break;
                            }

                            string AcutalMsgToSend = string(sendMessageBuffer, bytecnt - 1);
                            cout << AcutalMsgToSend << endl;

                            responseMessage.clear();
                            responseMessage = "s: 200 OK you have a new message from ";
                            responseMessage += currentLoggedinUserName;
                            responseMessage += "  ";
                            responseMessage += currentLoggedinUserName;
                            responseMessage += ": ";
                            responseMessage += AcutalMsgToSend;
                            cout << responseMessage << endl;
                            strcpy(sendMessageBuffer, responseMessage.c_str());
                            send(targetUserSocketNumber, sendMessageBuffer, strlen(sendMessageBuffer) + 1, 0);

                            responseMessage.clear();
                            responseMessage = server_sucess_message;
                            send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                            break;
                        }
                    } // SEND Command for john ends
                    else
                    {
                        responseMessage.clear();
                        responseMessage = "Invalid target username";
                        send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                        break;
                    }
                }                                                       // SEND Command for root ends
                else if (isSameNoCase(recievedCommand, logout_command)) // LOGOUT Command Begins
                {
                    string responseMessage = "Response from Server: No logged in user\n";
                    if (isUserLoggedIn)
                    {
                        isUserLoggedIn = false; // user is logged out, preventing user from using msgstore
                        SocketVSAddress.erase(childSocket);
                        SocketVsUserName.erase(childSocket);
                        responseMessage = server_sucess_message;
                    }
                    if (isRootUserLoggedIn)
                    {
                        isRootUserLoggedIn = false; // added condition that has to be changed if user is the root
                    }
                    send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                    break;
                }                                                     // LOGOUT Command Ends
                else if (isSameNoCase(recievedCommand, quit_command)) // QUIT Command Begins
                {
                    SocketVSAddress.erase(childSocket);
                    SocketVsUserName.erase(childSocket);
                    string responseMessage = server_sucess_message;
                    send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0); // simply sends 200 OK, termination process on client side
                    break;
                }                                                         // QUIT Command Ends
                else if (isSameNoCase(recievedCommand, shutdown_command)) // SHUTDOWN Command Begins
                {
                    string responseMessage;
                    if (isRootUserLoggedIn) // only can be used if the user is the root
                    {
                        // outputs message, closes socket, terminates server
                        responseMessage = shutdown_message;
                        for (auto &Socket : SocketVSAddress) // goes through each open client and closes the socket
                        {
                            send(Socket.first, responseMessage.c_str(), responseMessage.size() + 1, 0);
                            close(Socket.first);
                        }
                        cout << "System has shutdown\n";
                        serverShutdown = true;
                        exit(1);
                        break;
                    }
                    else
                    {
                        responseMessage = "402 User not allowed\n";
                        send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                        break;
                    }
                } // SHUTDOWN Command Ends
                else
                {
                    string responseMessage = "Enter a valid command\n"; // error message if user enters any other command
                    send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                    break;
                }
            }
        }
    }
}

int main(void)
{
    readAndStoreInitMessagesFromFile();
    struct sockaddr_in myaddr; // server address
                               // client address
    int newfd;                 // newly accept()ed socket descriptor
    int yes = 1;               // for setsockopt() SO_REUSEADDR, below
    socklen_t addrlen;
    char *temp;

    int i = 0;
    int n;

    pthread_t cThread;

    FD_ZERO(&master); // clear the master and temp sets

    // get the listener
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    // lose the pesky "address already in use" error message
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    // bind
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(SERVER_PORT);
    memset(&(myaddr.sin_zero), '\0', 8);
    if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    // listen
    if (listen(listener, 10) == -1)
    {
        perror("listen");
        exit(1);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    addrlen = sizeof(remoteaddr);

    // main loop
    for (;;)
    {
        // handle new connections
        if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen)) == -1)
        {
            perror("accept");
            exit(1);
        }
        else
        {
            FD_SET(newfd, &master); // add to master set

            cout << "multiThreadServer: new connection from "
                 << inet_ntoa(remoteaddr.sin_addr)
                 << " socket " << newfd << endl;
            SocketVSAddress[newfd] = string(inet_ntoa(remoteaddr.sin_addr));
            i++;
            if (newfd > fdmax)
            { // keep track of the maximum
                fdmax = newfd;
            }

            if (pthread_create(&cThread, NULL, ChildThread, (void *)newfd) < 0)
            {
                perror("pthread_create");
                exit(1);
            }
        }
    }
    return 0;
}
