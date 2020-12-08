/*
 * sclient.c
 * Program Made by: Hadi Daana and Johnathan Khalil
 * CIS-427 Jinhua Guo
 * Date: 11/5/2020
 */

#include <iostream>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <cstdlib>
#include <list>

using namespace std;

#define SERVER_PORT 1245
#define MAX_LINE 256
#define STDIN 0

string convertToString(char* a, int size) 
{
    string strng = a;
    return strng;
}

int main(int argc, char * argv[]) {

    fd_set master;   // master file descriptor list
    fd_set read_fds; // temp file descriptor list for select()
    int fdmax;       // maximum file descriptor number

    struct sockaddr_in sin;
    char buf[MAX_LINE];
    int len;
    int s;
    int size;

    string acc0 = "LOGIN ROOT ROOT01"; //users for acceptable logins
    string acc1 = "LOGIN JOHN JOHN01";
    string acc2 = "LOGIN DAVID DAVID01";
    string acc3 = "LOGIN MARY MARY01";

    string sendJ = "SEND JOHN"; //strings for SEND command
    string sendM = "SEND MARY";
    string sendD = "SEND DAVID";
    string sendR = "SEND ROOT";
    string error;

    string login = "LOGIN"; //Commands that require client side interaction
    string logout = "LOGOUT";
    string quit = "QUIT";
    string shutdown = "SHUTDOWN";
    string shutmsg;
    string msgstore = "MSGSTORE";
    string msgget = "MSGGET";
    string who = "WHO";
    bool loggedIn = false;
    
    

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    /* active open */
    if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("select client: socket");
		exit(1);
    }

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr  = inet_addr(argv[1]);
    sin.sin_port = htons (SERVER_PORT);
    
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
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

  while (1) {

          read_fds = master; // copy it
          if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
              perror("select");
              exit(1);
          }
          // looking for data to read either from the server or the user
          if (FD_ISSET(STDIN, &read_fds)) {
              // handle the user input
              
              if (fgets(buf, sizeof(buf), stdin)) {

                  for (int i = 0; i < MAX_LINE; i++) //Unifies all commands to uppercase so that the client can enter commands in both upper and lowercase without error
                  {
                      buf[i] = toupper(buf[i]);
                  }
                  buf[MAX_LINE - 1] = '\0';
                  len = strlen(buf) + 1;
                  send(s, buf, len, 0);
                  
                  if (strcmp(buf, sendJ.c_str()) == 10 || strcmp(buf, sendD.c_str()) == 10 || strcmp(buf, sendM.c_str()) == 10 || strcmp(buf, sendR.c_str()) == 10) //Allows program to unify all commands to uppercase without affecting the message entered by the user
                  {
                      recv(s, buf, sizeof(buf), 0);
                      cout << buf;
                      size = sizeof(buf) / sizeof(buf);
                      error = convertToString(buf, size);
                      if (error == "420 either the user does not exist or is not logged in\n") //Error condition if the user is sending to a someone that does not exist or not logged in
                      {
                          cout << " ";
                      }
                      else {
                          fgets(buf, sizeof(buf), stdin);
                          buf[MAX_LINE - 1] = '\0';
                          len = strlen(buf) + 1;
                          send(s, buf, len, 0);
                          recv(s, buf, sizeof(buf), 0);
                          cout << buf;
                      }
                  }if (strcmp(buf, acc0.c_str()) == 10 || strcmp(buf, acc1.c_str()) == 10 || strcmp(buf, acc2.c_str()) == 10 || strcmp(buf, acc3.c_str()) == 10)
                  {
                      loggedIn = true; //sets login condition to true to support lower if-statement for msgstore
                  }if (strcmp(buf, logout.c_str()) == 10)
                  {
                      loggedIn = false;
                  }if (strcmp(buf, msgstore.c_str()) == 10 && loggedIn == true && strcmp(buf, msgget.c_str()) != 10 && strcmp(buf, who.c_str()) != 10) //Allows program to unify all commands to uppercase without affecting the message entered by the user
                  {
                      recv(s, buf, sizeof(buf), 0);
                      cout << buf;
                      fgets(buf, sizeof(buf), stdin);
                      buf[MAX_LINE - 1] = '\0';
                      len = strlen(buf) + 1;
                      send(s, buf, len, 0);
                      recv(s, buf, sizeof(buf), 0);
                      cout << buf;

                  }if (strcmp(buf, quit.c_str()) == 10) //if the user quits, display message, close socket, terminate client
                  {
                      cout << "Client has terminated\n";
                      close(s);
                      exit(1);
                      break;
                  }



              }
              else {
                  break;
              }
          }
          if (FD_ISSET(s, &read_fds)) {
              // handle data from the server
	      
              if (recv(s, buf, sizeof(buf), 0) > 0) {
                  cout << buf;
                  size = sizeof(buf) / sizeof(buf); //closes and terminates the clients and server for shutdown function
                  shutmsg = convertToString(buf, size);
                  if (strcmp(buf, shutdown.c_str()) == 10 || shutmsg == "210 the server is about to shutdown......\n")
                  {
                      recv(s, buf, sizeof(buf), 0);
                      
                      close(s);
                      exit(1);
                      break;
                  }
                  
              }

              
          }
    }

    close(s);
}

