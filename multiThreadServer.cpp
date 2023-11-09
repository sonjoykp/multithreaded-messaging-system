#include "common.h"

fd_set master; // master file descriptor list
int listener;  // listening socket descriptor
int fdmax;
string client_addresses[10];
struct sockaddr_in remoteaddr;
list<string> Names;
list<string>::iterator it = Names.begin();

struct Node
{ // node list used for send and shutdown function
    string Data;
    string user;
    int Socket;
    struct Node *next;
};

Node Head;

string convertToString(char *a, int size)
{
    string strng = a;
    return strng;
}

string getUsername(string msg) // grabs the username from the login string
{
    char ptr;
    int x = 0;
    int i = 6;
    char userch[MAX_LINE];
    int size;
    string username;
    ptr = msg[i];
    while (ptr != ' ')
    {
        userch[x] = ptr;
        x++;
        i++;
        ptr = msg[i];
    }
    size = sizeof(userch) / sizeof(char);
    username = convertToString(userch, size);
    // username += 'D';

    return username;
}

void InsertNames(string name) // inserts usernames into a list (used for WHO function)
{
    for (int z = 0; z < name.length(); z++)
    {
        name[z] = tolower(name[z]);
    }
    Names.push_front(name);
}

void RemoveNames(string name) // removes usernames from list once logged out or quit
{
    for (int z = 0; z < name.length(); z++)
    {
        name[z] = tolower(name[z]);
    }
    Names.remove(name);
}

void AddNodes(string name, string user, int sockt) // adds nodes and socket numbers to a linked list, used in correlation with SEND
{
    Node *t = &Head;
    Node *P1 = new Node();
    P1->Data = name;
    P1->user = user;
    P1->Socket = sockt;
    while (t->next != NULL)
    {
        t = t->next;
    }
    t->next = P1;
}

void Send_Message(string name, string xperson, string msg)
{ // Sends message to client

    Node *m = &Head;
    int s;
    char g[MAX_LINE];
    string mg = "200 OK you have a new message from ";
    while (m != NULL)
    {
        if (m->Data == name)
        {
            cout << m->Socket;
            mg += xperson;
            mg += "\n";
            mg += xperson;
            mg += ": ";
            mg += msg;
            strcpy(g, mg.c_str());
            send(m->Socket, g, strlen(g) + 1, 0);
        }
        m = m->next;
    }
}

bool check_user(string name)
{
    Node *ptr = &Head;
    while (ptr != NULL)
    {
        if (ptr->Data == name)
        {
            return true;
        }
        if (ptr == NULL)
        {
            return false;
        }
        ptr = ptr->next;
    }

    return false;
}

void Send_Sender(int x) // Message notification on the sender side to inform of success
{
    char buf[MAX_LINE];
    string temp;
    temp = "200 OK\n";
    strcpy(buf, temp.c_str());
    send(x, buf, strlen(buf) + 1, 0);
}

// the child thread
void *ChildThread(void *newfd)
{
    char buf[MAX_LINE];
    char get[MAX_LINE];
    int nbytes;
    int i, j;
    int childSocket = (long)newfd;

    // string wh = "WHO";
    string sendJohnCommand = "SEND JOHN";
    string sendMaryCommand = "SEND MARY";
    string sendDavidCommand = "SEND DAVID";
    string sendRootCommand = "SEND ROOT";

    string david = "david"; // usernames stored in lowercase
    string mary = "mary";
    string root = "root";
    string john = "john";

    int size;
    int len;
    int s;
    string addmsg;
    string messages[20]; // stores the messages from txt file in array
    string temp;
    int itotal = 0; // limiter for number of messages stored
    int m = 0;
    char *temp1;
    int a = 0;
    string User;
    string usermsg;
    string Just_User;
    string msg;
    Node *hptr = &Head;
    Node *ptrshut = &Head; // used for shutdown function
    string lnames;

    bool loggedIn = false; // conditions
    bool shutdown = false;
    bool rootLogIn = false;

    string loginRootCommand = "login root root01";
    string loginJohnCommand = "login john john01";
    string loginDavidCommand = "login david david01";
    string loginMaryCommand = "login mary mary01";

    ifstream ifile;
    ofstream ofile;

    ifile.open("messages.txt");
    int f = 0;
    if (ifile.is_open()) // opens txt file and grabs each line from the file, stores that line in the array
    {
        getline(ifile, messages[f]);
        messages[f] += '\n';
        messages[f] += '\0';
        while (ifile)
        {
            ++f;
            ++itotal; // keeps track of the total number of messages stored
            getline(ifile, messages[f]);
            messages[f] += '\n';
            messages[f] += '\0';
        }
    }
    ifile.close();

    ofile.open("messages.txt", ios::app); // ensures that when msgstore is called, message added on newline
    ofile << "\n";
    ofile.close();

    while (!shutdown)
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
                if (isSameNoCase(recievedCommand, who_command)) // WHO Command Begins
                {
                    string responseMessage = "200 OK\n The list of active users: \n";
                    for (it = Names.begin(); it != Names.end(); it++)
                    {
                        responseMessage += " ";
                        responseMessage += *it;
                        responseMessage += "\n";
                    }
                    send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                    break;
                }                                                       // WHO Command Ends
                else if (isSameNoCase(recievedCommand, msgget_command)) // MSGGET Command Begins
                {
                    string responseMessage = "200 OK\n   ";
                    responseMessage += messages[m];
                    responseMessage += "\n";
                    m++;
                    if (m == itotal) // resets incrementer for msgget calls
                    {
                        m = 0;
                    }
                    send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                    break;

                }                                                             // MSGGET Command Ends
                else if (startsWithNoCase(recievedCommand, msgstore_command)) // MSGSTORE Command Begins
                {
                    string responseMessage;
                    if (loggedIn == false) // error condition if user is not logged in
                    {
                        responseMessage = "401 You are not currently logged in, login first";
                        responseMessage += "\n";
                        send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                        break;
                    }
                    if (loggedIn == true)
                    {
                        responseMessage = "200 OK\n";
                        responseMessage += "Enter a message of the day\n"; // added message for user friendliness
                        // sends 200 OK, then waits to  receive the user's message
                        send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                        recv(childSocket, get, sizeof(get), 0);
                        size = sizeof(get) / sizeof(char);
                        addmsg = string(get, size); // converts the message into a string and stores it in a variable

                        ofile.open("messages.txt", ios::app);
                        ofile << addmsg; // writes the user's message at the first available newline
                        ofile.close();
                        messages[itotal] = addmsg;
                        itotal++; // increments since there is a new total
                        cout << addmsg;
                        temp = "200 OK\n";
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0);
                        break;
                    } // MSGSTORE Command Ends
                }
                else if (startsWithNoCase(recievedCommand, login_command))
                {
                    if (isSameNoCase(recievedCommand, loginDavidCommand) || 
                        isSameNoCase(recievedCommand, loginJohnCommand) || 
                        isSameNoCase(recievedCommand, loginMaryCommand)) // LOGIN Command Begins
                    {
                        temp1 = inet_ntoa(remoteaddr.sin_addr); // grabs the ip addresses and stores in list
                        string temp2(temp1);
                        client_addresses[a] = temp2;
                        size = sizeof(buf) / sizeof(char);
                        usermsg = convertToString(buf, size);
                        Just_User += "SEND "; // accounts for SEND function
                        Just_User += getUsername(usermsg);
                        User = getUsername(usermsg);
                        lnames = User;
                        for (int z = 0; z < lnames.length(); z++) // lowercase the users
                        {
                            lnames[z] = tolower(lnames[z]);
                        }
                        AddNodes(Just_User, lnames, childSocket);
                        User += "\t";
                        User += client_addresses[a];
                        InsertNames(User); // insert usernames into list once logged in
                        it++;
                        a++;
                        temp = "200 OK\n";
                        loggedIn = true; // for users that are not the root
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0);
                        break;
                    }
                    else if (isSameNoCase(recievedCommand, loginRootCommand)) // if the user is the root
                    {
                        temp1 = inet_ntoa(remoteaddr.sin_addr);
                        string temp2(temp1);
                        client_addresses[a] = temp2;
                        size = sizeof(buf) / sizeof(char);
                        usermsg = convertToString(buf, size);
                        Just_User += "SEND ";
                        Just_User += getUsername(usermsg);
                        User = getUsername(usermsg);
                        lnames = User;
                        for (int z = 0; z < lnames.length(); z++)
                        {
                            lnames[z] = tolower(lnames[z]);
                        }
                        AddNodes(Just_User, lnames, childSocket);
                        User += "\t";
                        User += client_addresses[a];
                        InsertNames(User);
                        it++;
                        a++;
                        temp = "200 OK\n";
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0);
                        loggedIn = true;
                        rootLogIn = true;
                        break;
                    }
                    else // error case for any other users
                    {
                        temp = "410 Wrong UserID or Password\n";
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0);
                        break;
                    }
                } // LOGIN Command Ends
                else if (startsWithNoCase(recievedCommand, send_command))
                {
                    if (isSameNoCase(recievedCommand, sendJohnCommand))
                    {                                   // SEND Command for john begins (same logic for all cases)
                        if (check_user(sendJohnCommand) == false) // error condition check
                        {
                            temp = "420 either the user does not exist or is not logged in\n";
                            strcpy(buf, temp.c_str());
                            send(childSocket, buf, strlen(buf) + 1, 0);
                            break;
                        }
                        else
                        {
                            temp = "200 OK\n";
                            strcpy(buf, temp.c_str());
                            send(childSocket, buf, strlen(buf) + 1, 0);
                            recv(childSocket, get, sizeof(get), 0);
                            size = sizeof(get) / sizeof(char);
                            msg = convertToString(get, size);
                            hptr = &Head;
                            while (hptr != NULL)
                            {
                                if (hptr->Socket == childSocket) // identifies john in the list
                                {
                                    john = hptr->user;
                                }
                                hptr = hptr->next;
                            }
                            Send_Message(sendJohnCommand, john, msg); // sends john the message
                            Send_Sender(childSocket);       // sends the sender confirmation
                            break;
                        }
                    } // SEND Command for john ends
                    else if (isSameNoCase(recievedCommand, sendDavidCommand))
                    { // SEND command for david begins
                        if (check_user(sendDavidCommand) == false)
                        {
                            temp = "420 either the user does not exist or is not logged in\n";
                            strcpy(buf, temp.c_str());
                            send(childSocket, buf, strlen(buf) + 1, 0);
                            break;
                        }
                        else
                        {
                            temp = "200 OK\n";
                            strcpy(buf, temp.c_str());
                            send(childSocket, buf, strlen(buf) + 1, 0);
                            recv(childSocket, get, sizeof(get), 0);
                            size = sizeof(get) / sizeof(char);
                            msg = convertToString(get, size);
                            cout << msg;
                            hptr = &Head;
                            while (hptr != NULL)
                            {
                                if (hptr->Socket == childSocket)
                                {
                                    david = hptr->user;
                                }
                                hptr = hptr->next;
                            }
                            Send_Message(sendDavidCommand, david, msg);
                            Send_Sender(childSocket);
                            break;
                        }
                    } // SEND Command for david ends
                    else if (isSameNoCase(recievedCommand, sendMaryCommand))
                    { // SEND command for mary begins
                        if (check_user(sendMaryCommand) == false)
                        {
                            temp = "420 either the user does not exist or is not logged in\n";
                            strcpy(buf, temp.c_str());
                            send(childSocket, buf, strlen(buf) + 1, 0);
                            break;
                        }
                        else
                        {
                            temp = "200 OK\n";
                            strcpy(buf, temp.c_str());
                            send(childSocket, buf, strlen(buf) + 1, 0);
                            recv(childSocket, get, sizeof(get), 0);
                            size = sizeof(get) / sizeof(char);
                            msg = convertToString(get, size);
                            cout << msg;
                            hptr = &Head;
                            while (hptr != NULL)
                            {
                                if (hptr->Socket == childSocket)
                                {
                                    mary = hptr->user;
                                }
                                hptr = hptr->next;
                            }
                            Send_Message(sendMaryCommand, mary, msg);
                            Send_Sender(childSocket);
                            break;
                        }
                    } // SEND Command for mary ends
                    else if (isSameNoCase(recievedCommand, sendRootCommand))
                    { // SEND command for root begins
                        if (check_user(sendRootCommand) == false)
                        {
                            temp = "420 either the user does not exist or is not logged in\n";
                            strcpy(buf, temp.c_str());
                            send(childSocket, buf, strlen(buf) + 1, 0);
                            break;
                        }
                        else
                        {
                            temp = "200 OK\n";
                            strcpy(buf, temp.c_str());
                            send(childSocket, buf, strlen(buf) + 1, 0);
                            recv(childSocket, get, sizeof(get), 0);
                            size = sizeof(get) / sizeof(char);
                            msg = convertToString(get, size);
                            hptr = &Head;
                            while (hptr != NULL)
                            {
                                if (hptr->Socket == childSocket)
                                {
                                    root = hptr->user;
                                }
                                hptr = hptr->next;
                            }
                            Send_Message(sendRootCommand, root, msg);
                            Send_Sender(childSocket);
                            break;
                        }
                    }
                }                                                       // SEND Command for root ends
                else if (isSameNoCase(recievedCommand, logout_command)) // LOGOUT Command Begins
                {
                    string responseMessage = "Response from Server: No logged in user\n";
                    if (loggedIn == true)
                    {
                        loggedIn = false;  // user is logged out, preventing user from using msgstore
                        RemoveNames(User); // removes names from list once logged out
                        responseMessage = "200 OK\n";
                    }
                    if (rootLogIn)
                    {
                        rootLogIn = false; // added condition that has to be changed if user is the root
                        RemoveNames(User);
                        responseMessage = "200 OK\n";
                    }
                    send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                    break;
                }                                                     // LOGOUT Command Ends
                else if (isSameNoCase(recievedCommand, quit_command)) // QUIT Command Begins
                {
                    RemoveNames(User); // removes name from list once quit
                    string responseMessage = "200 OK\n";
                    // simply sends 200 OK, termination process on client side
                    send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                    break;
                }                                                         // QUIT Command Ends
                else if (isSameNoCase(recievedCommand, shutdown_command)) // SHUTDOWN Command Begins
                {
                    string responseMessage;
                    if (rootLogIn) // only can be used if the user is the root
                    {
                        // outputs message, closes socket, terminates server
                        responseMessage = shutdown_message;
                        ptrshut = &Head;
                        while (ptrshut != NULL) // goes through each open client and closes the socket
                        {
                            send(ptrshut->Socket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                            close(ptrshut->Socket);
                            ptrshut = ptrshut->next;
                        }
                        cout << "System has shutdown\n";
                        shutdown = true;
                        exit(1);
                        break;
                    }
                    else
                    {
                        responseMessage = "402 User not allowed\n";
                        send(childSocket, responseMessage.c_str(), responseMessage.size() + 1, 0);
                        break;
                    }
                    // SHUTDOWN Command Ends
                }
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
