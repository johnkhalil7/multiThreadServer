
README file

This project contains two programs; client and server, which work in coordination of
one another. The server program should be running before the client is executed.
The client program requires an IP address as a commandline argument of the server.
Multiple clients can connect to this thread

How to Compile:
1. Create a directory for the programs.
2. When creating a directory, make sure it has the following files:
	sclient.c
	multiThreadServer.c
	Makefile
3. Go to the directory in terminal and type in "make clean" command
4. Now type in "make" command
5. It should output: 
	g++ -g -c sclient.c
	g++  -o sclient sclient.o
	g++ -g -c multiThreadServer.c
	g++  -o multiThreadServer multiThreadServer.o

6. Apply these rules to both the client and server programs


How to run:
1. Open two window terminals
2. In the first terminal window, type in the command, "./multiThreadServer"
3. It should the send message: "The server is up, waiting for connection"
4. In the second terminal window, type in the next command: "./sclient 127.0.0.1" or "./sclient (whatever your ip address is using the curl ifconfig.me command)"
5. You can open a third terminal window and connect another client using the other option from step 4

Commands: 
1. msgget
This command from client sends a message to server to return the
message of the day. The server has access to file messages.txt and 
returns each message in that file in chronological order.
		 
2. shutdown
It will allow the user on the client side to shutdown the server.
ONLY ROOT has access to shutdown the server. Will display a message stating that the server
has shutdown.

3. login username password
It will allow the user at client side to log in to access the server.
The login command is followed by username and password. If the credentials
are authenticated, the user logs in, otherwise, it will display an error message.

4. logout
It will allow the logged in user to logout from the server

5. quit 
It will terminate the client connection from server. WIll display message indicating that client has been
terminated.

6. msgstore
It will allow the user to upload one message to the server. The server will save
the message to the messages.txt file. The limit for messages is 20.

7. who
It will output a list of current logged in users and their respective ip addresses

8. send
It will allow user to send a message to another specified logged in user

Sample Output for Client 1:
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
210 the server is about to shutdown......
----------------------------------------------------------------------------------------------------
Sample Output for Client 2:
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
210 the server is about to shutdown......

----------------------------------------------------------------------------------------------------
Sample Output for Server:

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



