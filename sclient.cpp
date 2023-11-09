

#include "common.h"

string convertToString(char *a, int size)
{
    string strng = a;
    return strng;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: client <Server IP Address>" << endl;
        exit(1);
    }

    fd_set master;   // master file descriptor list
    fd_set read_fds; // temp file descriptor list for select()
    int fdmax;       // maximum file descriptor number

    struct sockaddr_in sin;
    char buf[MAX_LINE];
    int len;
    int s;

    FD_ZERO(&master); // clear the master and temp sets
    FD_ZERO(&read_fds);

    /* active open */
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("select client: socket");
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(argv[1]);
    sin.sin_port = htons(SERVER_PORT);

    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("select client: connect");
        close(s);
        exit(1);
    }

    // add the STDIN to the master set
    FD_SET(STDIN, &master);

    // add the listener to the master set
    FD_SET(s, &master);

    // keep track of the biggest file descriptor
    fdmax = s; // so far, it's this one

    /* main loop; get and send lines of text */
    while (1)
    {
        read_fds = master; // copy it
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(1);
        }

        // looking for data to read either from the server or the user
        if (FD_ISSET(STDIN, &read_fds))
        {
            // handle the user input
            string inputCommand;
            getline(cin, inputCommand);
            send(s, inputCommand.c_str(), inputCommand.size() + 1, 0);
            if (isSameNoCase(inputCommand, quit_command))
            {
                close(s);
                exit(1);
                break;
            }
        }

        if (FD_ISSET(s, &read_fds))
        {
            int receivedbytes;
            // handle data from the server
            if ((receivedbytes = recv(s, buf, sizeof(buf), 0)) > 0)
            {
                string messageFromServer = string(buf, receivedbytes - 1);
                cout << messageFromServer;
                if (isSameNoCase(shutdown_message, messageFromServer))
                {
                    close(s);
                    exit(1);
                    break;
                }
            }
        }
    }

    close(s);
}
