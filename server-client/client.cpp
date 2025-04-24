#include "socket.h"

using namespace ktg;

int main(){
    Socket client;

    string ip = "127.0.0.1";
    int64_t port = 8080;
    client.sconnect(ip, port);

    string msg = "hello, server";
    client.sendMsg(msg.c_str(), msg.size());

    char buf[1024] = {0};
    client.recvMsg(buf, sizeof(buf));
    cout<<buf<<endl;

    client.sclose();
    
    return 0;
}