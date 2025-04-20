#include <iostream>
#include <algorithm>

#include "myVector.h"
using namespace ktg;


int main(){
    myVector<int>vec;
    for(int i = 0; i < 10; i++){
        vec.push_back(i);
    }
    
    for(auto it = vec.begin(); it != vec.end(); it++){
        std::cout<<*it<<" ";
    }
    std::cout<<std::endl;


    // vec.insert(vec.begin(),2,7);
    // for(auto it = vec.begin(); it != vec.end(); it++){
    //     std::cout<<*it<<" ";
    // }
    // std::cout<<std::endl;

    vec.erase(vec.begin() + 2, vec.begin() + 5);
    for(auto it = vec.begin(); it != vec.end(); it++){
        std::cout<<*it<<" ";
    }
    std::cout<<std::endl;

    return 0;
}