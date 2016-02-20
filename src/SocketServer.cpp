#include "SocketServer.h"

SocketServer::SocketServer(int port)
{
    this->port=port;

    WSAStartup(MAKEWORD(2,0), &WSAData);
    server = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    bind(server, (SOCKADDR *)&addr, sizeof(addr));
}

SOCKET SocketServer::accept()
{
    listen(server, 0);
    SOCKET client;
    SOCKADDR_IN clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    if((client = WINSOCK_API_LINKAGE::accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)
    {
        return client;
    }
    return -1;
}

void SocketServer::close()
{
    WINSOCK_API_LINKAGE::closesocket(server);
}
