
#include <iostream>
#include <algorithm>
using namespace std;

#include "json.h"
using namespace ktg;

void init(){
    Json v1;
    Json v2 = true; //Json v2(true)
    Json v3 = 10; //Json v3(10)
    Json v4 = 1.0;
    Json v5 = "ktg";

    bool bo = v2;
    int in = v3;
    double dob = v4;
    string str = v5;

    Json arr;
    arr[0] = true;
    arr[1] = 12;
    arr.append(1.0);
    arr.append("ktg");

    for(auto it = arr.begin(); it != arr.end(); it++){
        cout<<*(it)<<" ";
    }
    cout<<endl;
    
    // bool bo = arr[0];
    // int in = arr[1];
    // double dob = arr[2];
    // const string & str = arr[3];

    // cout<<bo<<" "<<in<<" "<<dob<<" "<<str<<endl;
}

int main(){ 
    Json obj;
    obj["bool"] = true;
    obj["int"] = 12;
    obj["double"] = 1.0;
    obj["str"] = "my dream";

    cout<<obj.str()<<endl;

    return 0;
}