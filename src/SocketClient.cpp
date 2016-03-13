#include "SocketClient.h"

SocketClient::SocketClient(std::string ip, int port)
{
    this->ip=ip;
    this->port=port;
    this->connected=false;
    initParameters();
    initSocket(ip, port);
}

SocketClient::SocketClient(SOCKET socket)
{
    this->socket=socket;
    this->connected=true;
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
    this->bytes_for_package_size=16;
    this->size_of_packages=2048;
    this->callback=NULL;
    this->callbackError=NULL;
    this->thread_started=false;
    this->errorWhileReceiving=false;
    this->errorWhileSending=false;
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
        connected=true;

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
    std::stringstream ss;
    ss << message;
    send(ss);
}

void SocketClient::send(std::istream &stream)
{
    char buffer[size_of_packages];
    int streamsize, p, result;
    std::stringstream ss;
    std::string strSize;

    stream.seekg(0, std::ios::end);
    streamsize = stream.tellg();
    stream.seekg(std::ios::beg);

    ss << streamsize;
    strSize = ss.str();

    while(strSize.size()<bytes_for_package_size)
    {
        strSize="0"+strSize;
    }

    result = WINSOCK_API_LINKAGE::send(socket, strSize.c_str(), bytes_for_package_size, 0);
    if(errorSending(result))
        return;

    for (unsigned int i=0 ; i<streamsize/size_of_packages ; i++)
    {
        stream.read(buffer, size_of_packages);
        result = WINSOCK_API_LINKAGE::send(socket, buffer, size_of_packages, 0);
        if(errorSending(result))
            return;

        memset(buffer, 0, size_of_packages);
    }

    p=streamsize%size_of_packages;
    if(p!=0)
    {
        char buff[p];
        stream.read(buff, p);
        result = WINSOCK_API_LINKAGE::send(socket, buff, p, 0);
        if(errorSending(result))
            return;
    }
}

bool SocketClient::errorSending(int result)
{
    if(result==SOCKET_ERROR)
    {
        if(callbackError!=NULL)
        {
            errorStruct error(*this, WSAGetLastError(), "Error while sending.");
            callbackError(&error);
        }
        connected=false;
        errorWhileSending=true;
        return true;
    }
    return false;
}

std::string SocketClient::receive()
{
    char buffer[bytes_for_package_size];
    int result = WINSOCK_API_LINKAGE::recv(socket, buffer, bytes_for_package_size, 0);

    if(errorReceiving(result))
        return "";

    std::string messageSize(buffer, bytes_for_package_size);
    std::stringstream ss;
    int n;

    ss << messageSize;
    ss >> n;

    Sleep(1);

    std::string message;
    for (unsigned int i=0 ; i<n/size_of_packages ; i++)
    {
        char* buff = new char[size_of_packages]();
        int result = WINSOCK_API_LINKAGE::recv(socket, buff, size_of_packages, 0);

        if(errorReceiving(result))
            return "";

        message+=std::string(buff, size_of_packages);
        delete[] buff;
    }

    if(n%size_of_packages!=0)
    {
        int p=n%size_of_packages;
        char* buff = new char[p]();
        int result = WINSOCK_API_LINKAGE::recv(socket, buff, p, 0);

        if(errorReceiving(result))
            return "";

        message+=std::string(buff, p);
        delete[] buff;
    }

    return message;
}

bool SocketClient::errorReceiving(int result)
{
    if(result==0 || result<0)
    {
        if(callbackError!=NULL)
        {
            errorStruct error(*this, WSAGetLastError(), "Receive failed.");
            callbackError(&error);
        }
        connected=false;
        errorWhileReceiving=true;
        return true;
    }
    return false;
}

void SocketClient::setSize_of_packages(unsigned int n)
{
    this->size_of_packages=n;
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
        if(connected)
        {
            std::string message = this->receive();
            if(errorWhileReceiving)
            {
                errorWhileReceiving=false;
            }
            else if(errorWhileSending)
            {
                errorWhileSending=false;
            }
            else
            {
                messageStruct m(*this, message);
                callback(&m);
            }
        }
    }
}

bool SocketClient::isConnected()
{
    return connected;
}
