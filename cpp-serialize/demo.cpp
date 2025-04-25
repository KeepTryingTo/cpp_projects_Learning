#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <map>

#include "DataStream.h"
using namespace ktg::serialize;

int main(){
    DataStream ds;
    set<int>st;
    for(int i = 0; i < 5; i++)st.insert(i);
    ds.write(st);

    set<int>ans;
    ds.read(ans);

    cout<<"size = "<<ans.size()<<endl;
    for(auto it = ans.begin(); it != ans.end(); it++){
        cout<<*it<<" ";    
    }
    cout<<endl;
    return 0;
}