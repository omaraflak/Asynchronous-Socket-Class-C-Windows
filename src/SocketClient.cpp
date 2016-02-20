#include "SocketClient.h"

SocketClient::SocketClient(std::string ip, int port)
{
    this->ip=ip;
    this->port=port;
    initParameters();
    initSocket(ip, port);
}

SocketClient::SocketClient(SOCKET socket)
{
    this->socket=socket;
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

void SocketClient::connect()
{
    WINSOCK_API_LINKAGE::connect(socket, (SOCKADDR *)&addr, sizeof(addr));
}

void SocketClient::close()
{
    if(thread_started)
        removeMessageCallback();
    removeErrorCallback();
    WINSOCK_API_LINKAGE::closesocket(socket);
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
    WINSOCK_API_LINKAGE::send(socket, message.c_str(), message.length(), 0);
}

std::string SocketClient::receive()
{
    char buffer[bytes_for_package_size];
    int result = WINSOCK_API_LINKAGE::recv(socket, buffer, sizeof(buffer), 0);

    if(result<0 || result==0){
        errorStruct error(*this, WSAGetLastError(), "Connection closed.");
        if(callbackError!=NULL)
            callbackError(&error);
        return "";
    }

    std::string messageSize(buffer, bytes_for_package_size);
    std::stringstream ss;
    ss << messageSize;
    int n;
    ss >> n;

    std::string message;

    for (unsigned int i=0 ; i<n/size_of_received_buffer ; i++)
    {
        char* buff = new char[size_of_received_buffer]();
        WINSOCK_API_LINKAGE::recv(socket, buff, size_of_received_buffer, 0);
        std::string str(buff, size_of_received_buffer);
        message+=str;
        delete[] buff;
    }

    if(n%size_of_received_buffer!=0)
    {
        int p=n%size_of_received_buffer;
        char* buff = new char[p]();
        WINSOCK_API_LINKAGE::recv(socket, buff, p, 0);
        std::string str(buff, p);
        message+=str;
        delete[] buff;
    }

    return message;
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
        std::string message = this->receive();
        messageStruct m(*this, message);
        callback(&m);
    }
}
