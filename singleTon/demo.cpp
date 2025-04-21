#include <iostream>
#include "classA.h"
#include "singleTon.h"
using namespace ktg;

int main(){
    // A * a = A::getInstance();
    // a -> display();
    A * a =  SingleTon<A>::getInstance();
    a -> display();
    return 0;
}