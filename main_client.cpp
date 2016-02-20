#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include "SocketClient.h"

using namespace std;

int main()
{
    SocketClient client("127.0.0.1", 5555);
    client.connect();

    string str;
    while(1)
    {
        cout << ">";
        getline(cin, str);
        client.send(str);
    }
    client.close();
}
