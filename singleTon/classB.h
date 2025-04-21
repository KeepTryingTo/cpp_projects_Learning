#pragma once

#include <iostream>
#include <algorithm>
#include <cstring>

#include "singleTon.h"

using namespace ktg;

namespace ktg {
    class B {
        friend class SingleTon<B>;
        public:
            void display(){
                std::cout<<"name = "<<m_name<<std::endl;
            }
        private:
            B():m_name ("ktg"){}
            // 正确的拷贝构造函数
            B(const B& other) : m_name(other.m_name) {
                std::cout << "Copy constructor called" << std::endl;
            }
            ~B(){};
            // 赋值运算符
            B& operator=(const B& other) {
                if (this != &other) {
                    m_name = other.m_name;
                }
                return *this;
            }
        private:
            std::string m_name;
    };
}

