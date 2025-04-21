#pragma once

#include <iostream>
#include <algorithm>
#include <cstring>

#include "singleTon.h"

using namespace ktg;

namespace ktg {
    class A {
        friend class SingleTon<A>;
        public:
            void display(){
                std::cout<<"name = "<<m_name<<std::endl;
            }
        private:
            A():m_name ("ktg"){}
            // 正确的拷贝构造函数
            A(const A& other) : m_name(other.m_name) {
                std::cout << "Copy constructor called" << std::endl;
            }
            ~A(){};
            // 赋值运算符
            A& operator=(const A& other) {
                if (this != &other) {
                    m_name = other.m_name;
                }
                return *this;
            }
        private:
            std::string m_name;
    };
}

