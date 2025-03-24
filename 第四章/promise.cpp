#include <iostream>
#include <future>
#include <string>
#include <thread>
#include <algorithm>
#include <map>

using payload_type = std::string;

class connection {
    public:
        std::map<int,std::promise<payload_type>>promises;
        std::future<payload_type> get_future(int id){
            std::promise<payload_type>promise;
            //对期值给出一个承诺
            std::future<payload_type> future = promise.get_future();
            promises[id] = std::move(promise);
            return future;
        }

        void send(const payload_type & data, int id){
            std::cout<<"Data id = "<<id<<" data = "<<data<<std::endl;
            if(promises.count(id) > 0){
                //前面已经给出了承诺，这里将会给出数据
                promises[id].set_value(data);
            }else{
                std::cout<<"No promise found for ID"<<id<<std::endl;
            }
        }
};

int main(){
    connection conn;
    auto future1 = conn.get_future(1);
    auto future2 = conn.get_future(2);
    //开始兑现承诺
    std::thread sender1([&](){
        conn.send("this is value 1", 1);
    });
    std::thread sender2([&](){
        conn.send("this is value 2", 2);
    });

    sender1.join();
    sender2.join();

    std::cout<<"recv data 1"<<future1.get()<<std::endl;
    conn.promises.erase(conn.promises.find(1));

    std::cout<<"recv data 2"<<future2.get()<<std::endl;
    conn.promises.erase(conn.promises.find(2));
    return 0;
}