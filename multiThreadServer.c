/*
 * multiThreadServer.c -- a multithreaded server
 * Program Made by: Hadi Daana and Johnathan Khalil
 * CIS-427 Jinhua Guo
 * Date: 11/5/2020
 */


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <netdb.h>
#include <string>
#include <list>

using namespace std;

#define PORT 1245  // port we're listening on
#define MAX_LINE 256

fd_set master;   // master file descriptor list
int listener;    // listening socket descriptor
int fdmax;
string client_addresses[10];
struct sockaddr_in remoteaddr;
list<string> Names;
list<string>::iterator it = Names.begin();


struct Node { //node list used for send and shutdown function
    string Data;
    string user;
    int Socket;
    struct Node* next;
};

Node Head;

string convertToString(char* a, int size)
{
    string strng = a;
    return strng;
}

string getUsername(string msg) //grabs the username from the login string
{
    char ptr;
    int x = 0;
    int i = 6;
    char userch[MAX_LINE];
    int size;
    string username;
    ptr = msg[i];
    while(ptr != ' ')
    {
        userch[x] = ptr;
        x++;
        i++;
        ptr = msg[i];
    }
    size = sizeof(userch) / sizeof(char);
    username = convertToString(userch, size);
    //username += 'D';
    
    return username;
}

void InsertNames(string name) //inserts usernames into a list (used for WHO function)
{
    for (int z = 0; z < name.length(); z++)
    {
        name[z] = tolower(name[z]);
    }
    Names.push_front(name);
}

void RemoveNames(string name) //removes usernames from list once logged out or quit
{
    for (int z = 0; z < name.length(); z++)
    {
        name[z] = tolower(name[z]);
    }
    Names.remove(name);
          
}

void AddNodes(string name,string user, int sockt) //adds nodes and socket numbers to a linked list, used in correlation with SEND
{
    Node* t = &Head;
    Node* P1 = new Node();
    P1->Data = name;
    P1->user = user;
    P1->Socket = sockt;
    while (t->next != NULL)
    {
        t = t->next;
    }
    t->next = P1;
}

void Send_Message(string name,string xperson ,string msg) { //Sends message to client

    Node* m = &Head;
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
    Node* ptr = &Head;
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
}

void Send_Sender(int x) //Message notification on the sender side to inform of success
{
    char buf[MAX_LINE];
    string temp;
    temp = "200 OK\n";
    strcpy(buf, temp.c_str());
    send(x, buf, strlen(buf) + 1, 0);
}


// the child thread
void *ChildThread(void *newfd) {
    char buf[MAX_LINE];
    char get[MAX_LINE];
    int nbytes;
    int i, j;
    int childSocket = (long) newfd;
    string login = "LOGIN"; //user commands
    string logout = "LOGOUT";
    string quit = "QUIT";
    string msgget = "MSGGET";
    string msgstore = "MSGSTORE";
    string stdown = "SHUTDOWN";
    string wh = "WHO";
    string sendJ = "SEND JOHN";
    string sendM = "SEND MARY";
    string sendD = "SEND DAVID";
    string sendR = "SEND ROOT";

    string david = "david"; //usernames stored in lowercase
    string mary = "mary";
    string root = "root";
    string john = "john";

    int size;
    int len;
    int s;
    string addmsg;
    string messages[20]; //stores the messages from txt file in array
    string temp;
    int itotal = 0; //limiter for number of messages stored
    int m = 0;
    char* temp1;
    int a = 0;
    string User;
    string usermsg;
    string Just_User;
    string msg;
    Node* hptr = &Head;
    Node* ptrshut = &Head; //used for shutdown function
    string lnames;

    bool loggedIn = false; //conditions
    bool shutdown = false;
    bool rootLogIn = false;

    string acc0 = "LOGIN ROOT ROOT01"; //users for acceptable logins
    string acc1 = "LOGIN JOHN JOHN01";
    string acc2 = "LOGIN DAVID DAVID01";
    string acc3 = "LOGIN MARY MARY01";


    ifstream ifile;
    ofstream ofile;

    ifile.open("messages.txt");
    int f = 0;
    if (ifile.is_open()) //opens txt file and grabs each line from the file, stores that line in the array
    {
        getline(ifile, messages[f]);
        messages[f] += '\n';
        messages[f] += '\0';
        while (ifile)
        {
            ++f;
            ++itotal; //keeps track of the total number of messages stored
            getline(ifile, messages[f]);
            messages[f] += '\n';
            messages[f] += '\0';
        }
    }
    ifile.close();

    ofile.open("messages.txt", ios::app); //ensures that when msgstore is called, message added on newline
    ofile << "\n";
    ofile.close();

    while (!shutdown)
    {
        while (1)
        {
            // handle data from a client
            if ((nbytes = recv(childSocket, buf, sizeof(buf), 0)) <= 0) {
                // got error or connection closed by client
                if (nbytes == 0) {
                    // connection closed
                    cout << "multiThreadServer: socket " << childSocket << " hung up" << endl;
                }
                else {
                    perror("recv");
                }
                close(childSocket); // bye!
                FD_CLR(childSocket, &master); // remove from master set
                pthread_exit(0);
            }
            else
            {
                cout << buf;
                 if (strcmp(buf, wh.c_str()) == 10) //WHO Command Begins
                 {
                    temp = "200 OK\n The list of active users: \n";
                    strcpy(buf, temp.c_str());
                    for (it = Names.begin(); it != Names.end(); it++)
                    {
                        temp += " ";
                        temp += *it;
                        temp += "\n";
                        strcpy(buf, temp.c_str());
                    }
                    send(childSocket, buf, strlen(buf) + 1, 0);
                    break;
                 } //WHO Command Ends
                if (strcmp(buf, msgget.c_str()) == 10) //MSGGET Command Begins
                {
                    temp = "200 OK\n   ";
                    temp += messages[m];
                    temp += "\n";
                    strcpy(buf, temp.c_str());
                    m++;
                    if (m == itotal) //resets incrementer for msgget calls
                    {
                        m = 0;
                    }
                    send(childSocket, buf, strlen(buf) + 1, 0);
                    break;
                    
                }//MSGGET Command Ends  
                if (strcmp(buf, msgstore.c_str()) == 10) //MSGSTORE Command Begins
                {
                    if (loggedIn == false) //error condition if user is not logged in
                    {
                        temp = "401 You are not currently logged in, login first";
                        temp += "\n";
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0);
                        break;
                    }
                    if (loggedIn == true)
                    {
                        temp = "200 OK\n";
                        temp += "Enter a message of the day\n"; //added message for user friendliness
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0); //sends 200 OK, then waits to  receive the user's message
                        recv(childSocket, get, sizeof(get), 0);
                        size = sizeof(get) / sizeof(char);
                        addmsg = convertToString(get, size); //converts the message into a string and stores it in a variable

                        ofile.open("messages.txt", ios::app);
                        ofile << addmsg; //writes the user's message at the first available newline
                        ofile.close();
                        messages[itotal] = addmsg;
                        itotal++; //increments since there is a new total
                        cout << addmsg;
                        temp = "200 OK\n";
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0);
                        break;
                    } //MSGSTORE Command Ends

                }
                if (strcmp(buf, acc1.c_str()) == 10 || strcmp(buf, acc2.c_str()) == 10 || strcmp(buf, acc3.c_str()) == 10)//LOGIN Command Begins
                {
                    temp1 = inet_ntoa(remoteaddr.sin_addr); //grabs the ip addresses and stores in list
                    string temp2(temp1);
                    client_addresses[a] = temp2;
                    size = sizeof(buf) / sizeof(char);
                    usermsg = convertToString(buf, size);
                    Just_User += "SEND "; //accounts for SEND function
                    Just_User += getUsername(usermsg);
                    User = getUsername(usermsg);
                    lnames = User;
                    for (int z = 0; z < lnames.length(); z++) //lowercase the users
                    {
                        lnames[z] = tolower(lnames[z]);
                    }
                    AddNodes(Just_User, lnames, childSocket);
                    User += "\t";
                    User += client_addresses[a];
                    InsertNames(User); //insert usernames into list once logged in
                    it++;
                    a++;
                    temp = "200 OK\n";
                    loggedIn = true; //for users that are not the root
                    strcpy(buf, temp.c_str());
                    send(childSocket, buf, strlen(buf) + 1, 0);
                    break;
                }
                if (strcmp(buf, acc0.c_str()) == 10) //if the user is the root
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
                    AddNodes(Just_User, lnames,childSocket);
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
                if (strcmp(buf, login.c_str()) == 32) //error case for any other users
                {
                    temp = "410 Wrong UserID or Password\n";
                    strcpy(buf, temp.c_str());
                    send(childSocket, buf, strlen(buf) + 1, 0);
                    break;

                }//LOGIN Command Ends
                if(strcmp(buf, sendJ.c_str()) == 10){ //SEND Command for john begins (same logic for all cases)
                    if (check_user(sendJ) == false) //error condition check
                    {
                        temp = "420 either the user does not exist or is not logged in\n";
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0); 
                        break;
                    }
                    else {
                        temp = "200 OK\n";
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0);
                        recv(childSocket, get, sizeof(get), 0);
                        size = sizeof(get) / sizeof(char);
                        msg = convertToString(get, size);
                        hptr = &Head;
                        while (hptr != NULL) 
                        {
                            if (hptr->Socket == childSocket) //identifies john in the list 
                            {
                                john = hptr->user;
                            }
                            hptr = hptr->next;
                        }
                        Send_Message(sendJ, john, msg); //sends john the message
                        Send_Sender(childSocket); //sends the sender confirmation
                        break;
                    }
                } //SEND Command for john ends
                if (strcmp(buf, sendD.c_str()) == 10) { //SEND command for david begins
                    if (check_user(sendD) == false)
                    {
                        temp = "420 either the user does not exist or is not logged in\n";
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0);
                        break;
                    }
                    else {
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
                        Send_Message(sendD, david, msg);
                        Send_Sender(childSocket);
                        break;
                    }
                } //SEND Command for david ends
                if (strcmp(buf, sendM.c_str()) == 10) {//SEND command for mary begins
                    if (check_user(sendM) == false)
                    {
                        temp = "420 either the user does not exist or is not logged in\n";
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0);
                        break;
                    }
                    else {
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
                        Send_Message(sendM, mary, msg);
                        Send_Sender(childSocket);
                        break;
                    }
                }//SEND Command for mary ends
                if (strcmp(buf, sendR.c_str()) == 10) { //SEND command for root begins
                    if (check_user(sendR) == false)
                    {
                        temp = "420 either the user does not exist or is not logged in\n";
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0);
                        break;
                    }
                    else {
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
                        Send_Message(sendR, root, msg);
                        Send_Sender(childSocket);
                        break;
                    }
                }//SEND Command for root ends
                if (strcmp(buf, logout.c_str()) == 10) //LOGOUT Command Begins
                {
                    if (loggedIn == true)
                    {
                        loggedIn = false; //user is logged out, preventing user from using msgstore
                        RemoveNames(User); //removes names from list once logged out
                        temp = "200 OK\n";
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0);
                    }
                    if (rootLogIn)
                    {
                        rootLogIn = false; //added condition that has to be changed if user is the root
                        RemoveNames(User);
                    }
                    break;
                } //LOGOUT Command Ends

                if (strcmp(buf, quit.c_str()) == 10) //QUIT Command Begins
                {
                    RemoveNames(User); //removes name from list once quit
                    temp = "200 OK\n";
                    strcpy(buf, temp.c_str()); //simply sends 200 OK, termination process on client side 
                    send(childSocket, buf, strlen(buf) + 1, 0);
                    break;
                } //QUIT Command Ends

                if (strcmp(buf, stdown.c_str()) == 10) //SHUTDOWN Command Begins
                {
                    if (rootLogIn) //only can be used if the user is the root
                    {
                        temp = "210 the server is about to shutdown......\n";
                        strcpy(buf, temp.c_str());
                        ptrshut = &Head;
                        while (ptrshut != NULL) //goes through each open client and closes the socket
                        {
                            send(ptrshut->Socket, buf, strlen(buf) + 1, 0);
                            close(ptrshut->Socket);
                            ptrshut = ptrshut->next;
                        }
                        cout << "System has shutdown\n";
                        shutdown = true;
                         //outputs message, closes socket, terminates server
                        exit(1);
                        break;
                    }
                    else {
                        temp = "402 User not allowed\n";
                        strcpy(buf, temp.c_str());
                        send(childSocket, buf, strlen(buf) + 1, 0);
                        break;
                    }
                    //SHUTDOWN Command Ends
                }
                else {
                    temp = "Enter a valid command\n"; //error message if user enters any other command
                    strcpy(buf, temp.c_str());
                    send(childSocket, buf, strlen(buf) + 1, 0);
                    break;
                }
            }

        }
    }
}


int main(void)
{
    struct sockaddr_in myaddr;     // server address
     // client address
    int newfd;        // newly accept()ed socket descriptor
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    socklen_t addrlen;
    char* temp;
    
    int i = 0;
    int n;

    pthread_t cThread;

    FD_ZERO(&master);    // clear the master and temp sets

    // get the listener
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // lose the pesky "address already in use" error message
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    // bind
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons(PORT);
    memset(&(myaddr.sin_zero), '\0', 8);
    if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(1);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    addrlen = sizeof(remoteaddr);
    
    // main loop
    for(;;) {
        // handle new connections
        if ((newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen)) == -1) {
            perror("accept");
	        exit(1);
        } else {
            FD_SET(newfd, &master); // add to master set
            
            cout << "multiThreadServer: new connection from "
		 		 << inet_ntoa(remoteaddr.sin_addr)
                 << " socket " << newfd << endl;
            i++;
            if (newfd > fdmax) {    // keep track of the maximum
                fdmax = newfd;
            }

	        if (pthread_create(&cThread, NULL, ChildThread, (void *) newfd) <0) {
                perror("pthread_create");
                exit(1);
            }
        }


    }
    return 0;
}

