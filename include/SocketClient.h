#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <string>
#include <sstream>
#include <winsock2.h>

struct messageStruct;
struct errorStruct;

class SocketClient
{
    public:
        SocketClient(std::string ip, int port);
        SocketClient(SOCKET socket);
        SocketClient();

        int connect();
        void close();
        void send(std::string message);
        std::string receive();
        void setSize_of_packages(unsigned int n);
        void setBytes_for_package_size(unsigned int n);
        void setMessageCallback(void (*callback_function)(messageStruct *));
        void setErrorCallback(void (*callback_function)(errorStruct *));
        void removeMessageCallback();
        void removeErrorCallback();
        bool isConnected();

    private:
        int port;
        std::string ip;
        unsigned int bytes_for_package_size;
        unsigned int size_of_packages;
        bool connected;
        WSADATA WSAData;
        SOCKET socket;
        SOCKADDR_IN addr;
        HANDLE thread;
        bool thread_started;
        bool errorWhileReceiving;
        bool errorWhileSending;
        void (*callback)(messageStruct *);
        void (*callbackError)(errorStruct *);
        bool errorReceiving(int result);
        bool errorSending(int result);
        void initSocket(std::string ip, int port);
        void initParameters();
        void startThread();

        static DWORD messageThread(LPVOID param) {
            SocketClient *client = (SocketClient*)param;
            client->startThread();
            return 0;
        }
};

struct messageStruct
{
    SocketClient client;
    std::string message;
    messageStruct(SocketClient client, std::string message) : client(client), message(message) {}
};

struct errorStruct
{
    SocketClient client;
    int code;
    std::string message;
    errorStruct(SocketClient client, int code, std::string message) : client(client), code(code), message(message) {}
};

#endif // SOCKETCLIENT_H
