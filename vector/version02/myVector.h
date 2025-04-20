#pragma once 

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include "vector_iterator.h"
#include "vector_const_iterator.h"
#include "vector_reserve_iterator.h"
using namespace ktg;


namespace ktg {
    template<typename T>
    class myVector{
        public:
            typedef vectorIterator<T>iterator;
            typedef vectorConstIterator<T>const_iterator;
            typedef vectorReserveIterator<T>reserve_iterator;
        public:
            myVector();
            ~myVector();

            T & at(int index);
            const T & at(int index)const; 

            //访问第一个元素和最后一个元素
            T & front();
            const T & front()const;
            T & back();
            const T & back()const;

            T * data();
            const T * data()const;

            T & operator[] (int index);
            const T & operator[] (int index)const;

            void push_back(const T & value);
            void pop_back();

            void assign(int n, const T & val);
            void swap(myVector<T>&other);

            bool empty()const;
            int size()const;
            int capacity()const;

            //改变数组中实际元素的数量
            void resize(int n);
            void resize(int n, const T & val);
            //改变容量大小
            void reserve(int n);
            void clear();

            //迭代器
            iterator begin();
            iterator end();

            iterator insert(iterator pos, const T & value);
            iterator insert(iterator pos, int n, const T & value);

            iterator erase(iterator pos);
            iterator erase(iterator first, iterator last);


            //常量迭代器
            const_iterator cbegin()const;
            const_iterator cend()const;

            //反向迭代器
            reserve_iterator rbegin();
            reserve_iterator rend();

            void display() const;

        private:
            bool is_basic_type();

        private:
            T * m_data;
            size_t m_size;
            size_t m_capacity;
    };

    #include "myVector.inl"
}

