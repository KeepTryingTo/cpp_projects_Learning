
#include <iostream>
#include <algorithm>
#include <cstring>

using namespace std;

#include "Logger.h"
using namespace ktg::utility;

int main(){
    Logger * log = Logger::getInstance();
    log -> open("./log.txt");
    //设置日志级别，在info以下的日志将会被丢弃掉，只会记录info及以上
    log -> setLevel(Logger::INFO);
    log -> set_log_max(200);

    debug("keep trying go");
    
    return 0;
}