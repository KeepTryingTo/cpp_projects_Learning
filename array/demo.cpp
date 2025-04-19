#include <iostream>
#include <algorithm>

#include "array.h"
using namespace ktg;

int main(){
    const int N = 10;
    Array<int,N>arr;
    for(int i = 0; i < N; i++){
        arr[i] = i;
    }
    std::cout<<arr.front()<<std::endl;

    for(auto it = arr.begin(); it != arr.end(); it++){
        std::cout<<*it<<" ";
    }
    std::cout<<std::endl;

    for(auto it = arr.rbegin(); it != arr.rend(); it++){
        std::cout<<*it<<" ";
    }
    std::cout<<std::endl;
    return 0;
}