#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <winsock2.h>
#include <string>
#include <sstream>

class SocketServer
{
    public:
        SocketServer(int port);
        SOCKET accept();
        void close();
    protected:
    private:
        int port;
        WSADATA WSAData;
        SOCKET server;
        SOCKADDR_IN addr;
};

#endif // SOCKETSERVER_H
