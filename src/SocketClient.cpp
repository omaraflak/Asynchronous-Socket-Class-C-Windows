#include "SocketClient.h"

SocketClient::SocketClient(std::string ip, int port)
{
    this->ip=ip;
    this->port=port;
    this->isConnected=false;
    initParameters();
    initSocket(ip, port);
}

SocketClient::SocketClient(SOCKET socket)
{
    this->socket=socket;
    this->isConnected=true;
    initParameters();
}

SocketClient::SocketClient(){}

void SocketClient::initSocket(std::string ip, int port)
{
    WSAStartup(MAKEWORD(2,0), &WSAData);
    this->socket = WINSOCK_API_LINKAGE::socket(AF_INET, SOCK_STREAM, 0);
    this->addr.sin_addr.s_addr = inet_addr(ip.c_str());
    this->addr.sin_family = AF_INET;
    this->addr.sin_port = htons(port);
}

void SocketClient::initParameters()
{
    this->bytes_for_package_size=10;
    this->size_of_received_buffer=2048;
    this->callback=NULL;
    this->callbackError=NULL;
    this->thread_started=false;
}

int SocketClient::connect()
{
    int r = WINSOCK_API_LINKAGE::connect(socket, (SOCKADDR *)&addr, sizeof(addr));
    if(r==SOCKET_ERROR)
    {
        if(callbackError!=NULL)
        {
            errorStruct error(*this, WSAGetLastError(), "Error while connecting.");
            callbackError(&error);
        }
    }
    else
        isConnected=true;

    return r;
}

void SocketClient::close()
{
    if(thread_started)
        removeMessageCallback();
    removeErrorCallback();
    WINSOCK_API_LINKAGE::closesocket(socket);
    WSACleanup();
}

void SocketClient::send(std::string message)
{
    int s = message.length();
    std::stringstream ss;
    ss << s;
    std::string str = ss.str();
    while(str.length()<bytes_for_package_size)
    {
        str="0"+str;
    }
    message=str+message;
    int r = WINSOCK_API_LINKAGE::send(socket, message.c_str(), message.length(), 0);
    if(r==SOCKET_ERROR)
    {
        if(callbackError!=NULL)
        {
            errorStruct error(*this, WSAGetLastError(), "Error while sending.");
            callbackError(&error);
        }
    }
}

std::string SocketClient::receive()
{
    char buffer[bytes_for_package_size];
    int result = WINSOCK_API_LINKAGE::recv(socket, buffer, bytes_for_package_size, 0);

    if(errorReceiving(result))
        return "NULL";

    std::string messageSize(buffer, bytes_for_package_size);
    std::stringstream ss;
    int n;

    ss << messageSize;
    ss >> n;

    std::string message;
    for (unsigned int i=0 ; i<n/size_of_received_buffer ; i++)
    {
        char* buff = new char[size_of_received_buffer]();
        int result = WINSOCK_API_LINKAGE::recv(socket, buff, size_of_received_buffer, 0);

            if(errorReceiving(result))
        return "NULL";

        message+=std::string(buff);
        delete[] buff;
    }

    if(n%size_of_received_buffer!=0)
    {
        int p=n%size_of_received_buffer;
        char* buff = new char[p]();
        int result = WINSOCK_API_LINKAGE::recv(socket, buff, p, 0);

            if(errorReceiving(result))
        return "NULL";

        std::string str(buff, p);
        message+=str;
        delete[] buff;
    }

    return message;
}

bool SocketClient::errorReceiving(int result)
{
    if(result==0 || result<0)
    {
        errorStruct error(*this, WSAGetLastError(), "Receive failed.");
        if(callbackError!=NULL)
            callbackError(&error);

        isConnected=false;
        return true;
    }
    return false;
}

void SocketClient::setSize_of_received_buffer(unsigned int n)
{
    this->size_of_received_buffer=n;
}

void SocketClient::setBytes_for_package_size(unsigned int n)
{
    this->bytes_for_package_size=n;
}

void SocketClient::setMessageCallback(void (*callback_function)(messageStruct *))
{
    callback = callback_function;
    thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)messageThread, this, 0, 0);
    thread_started=true;
}

void SocketClient::removeMessageCallback()
{
    CloseHandle(thread);
    callback = NULL;
    thread_started=false;
}

void SocketClient::setErrorCallback(void (*callback_function)(errorStruct *))
{
    callbackError = callback_function;
}

void SocketClient::removeErrorCallback()
{
    callbackError = NULL;
}

void SocketClient::startThread()
{
    while(1)
    {
        if(isConnected)
        {
            std::string message = this->receive();
            messageStruct m(*this, message);
            callback(&m);
        }
    }
}
