#include <iostream>
#include <future>
#include <string>
#include <thread>

void task_function(std::promise<void>promise){
    try{
        throw std::logic_error("foo");
    }catch(const std::exception & e){
        promise.set_exception(std::current_exception());
    }
    // promise.set_exception(std::make_exception_ptr(std::logic_error("foo")));
}

int main(){
    std::promise<void>some_promise;
    //承诺会给出值
    std::future<void>future = some_promise.get_future();
    std::thread task_thread(task_function, std::move(some_promise));
    task_thread.join();
    try {
        future.get();
    }catch (const std::exception & e){
        std::cout<<"exception = "<<e.what()<<std::endl;
    }
    return 0;
}