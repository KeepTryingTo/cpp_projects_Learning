#include <iostream>
#include <algorithm>

#include "list.h"
using namespace ktg;

int main(){
    List<int>list;
    for(int i = 1; i <= 10; i++){
        list.push_front(i);
    }   

    for(auto it = list.begin(); it != list.end(); it++){
        std::cout<<*it<<" ";
    }
    std::cout<<std::endl;

    list.insert(list.begin() + 2, 3,6);
    for(auto it = list.begin(); it != list.end(); it++){
        std::cout<<*it<<" ";
    }
    std::cout<<std::endl;
    return 0;
}