#include <iostream>
#include <fstream>
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
    
    /* NEW */               //  You can now send streams this way
    
    ifstream file("/path/to/file");
    client.send(file);      // The server will receive the STRING CONTENT of the file.
                            // But I'm working on a new version to receive the whole file directly (not as a string)
    /* NEW */
    
    client.close();
}
