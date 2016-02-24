# Why
Native socket implementation in C++ is... how to say... A TOTAL BRAINF***
If you never tryed to use them, don't try now!
SocketClient allows easy communication between sockets and implements a callback pattern for receiving messages!

# Callback Pattern
The SocketClient class implements a callback pattern which is very usefull to handle messages and catch errors easily.

For those who don't know: A callback is a function called when a specific event happen in the system. In this case, the callback will be called for every incoming message through the socket. You don't have to care about receiving messages, you are automatically 
alerted when you receive something.

# Sample

I made a simple Server/Client connection for the example: The Client connects to the server and can send as many messages as he wants.
The Server implements the callback functions and displays the messages.

Server: https://github.com/omaflak/Asynchronous-Socket-Class-C-Windows/blob/master/main_server.cpp

Client: https://github.com/omaflak/Asynchronous-Socket-Class-C-Windows/blob/master/main_client.cpp

# Compilation

    g++ main_client.cpp -o client.exe libSocket.a "C:\path\to\lib\libws2_32.a"
    g++ main_server.cpp -o server.exe libSocket.a "C:\path\to\lib\libws2_32.a"

SocketClient.h and SocketServer.h must be with YOUR .cpp files, i.e in the same folder.

# Tutorial

For more explanations about the installation, please visit my website: https://causeyourestuck.io/2016/02/21/socket-library-c/
