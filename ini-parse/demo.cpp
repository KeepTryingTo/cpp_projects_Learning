#include <iostream>
#include <algorithm>

#include "inifile.h"

using namespace ktg;


int main(){
    IniFile ini;
    ini.load("demo.ini");

    string ip = ini.get("server", "ip");
    int port = ini.get("server","port");
    cout<<ip<<" "<<port<<endl;

    ini.set("server", "timeout", 12);
    int timeout = ini.get("server","timeout");
    cout<<timeout<<endl;

    ini.display();

    ini.save("save.ini");
    return 0;
}