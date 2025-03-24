#include <future>
#include <iostream>
#include <algorithm>

using namespace std;

int fn(){
    int x = 6;
    return x;
}

struct node {
    int x;
    node(int x_): x(x_){}
    int find(int y){
        return y * (this -> x);
    }
};

int main(){
    future<int> the_fn = async(fn);
    cout<<"fn = "<<the_fn.get()<<endl;

    node nod(6);
    future<int> the_find = async(node::find, &nod, 6);
    cout<<"find = "<<the_find.get()<<endl;
    return 0;
}