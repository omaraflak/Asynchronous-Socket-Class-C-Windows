#include <iostream>
#include <winsock2.h>
#include "SocketClient.h"

using namespace std;

void onError(errorStruct *e)
{
    cout << e->message << endl;
}

int main()
{
    SocketClient client("127.0.0.1", 5555);
    client.setErrorCallback(onError);
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
