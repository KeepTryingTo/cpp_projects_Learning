
#pragma once 

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <cstring>

#include "array_iterator.h"
#include "array_const_iterator.h"
#include "array_reserve_iterator.h"
using namespace ktg;

namespace ktg {
    template<typename T, size_t N>
    class Array{
        public:
            typedef arrayIterator<T>iterator;
            typedef arrayConstIterator<T>const_iterator;
            typedef arrayReserveIterator<T>reserve_iterator;
        public:
            Array();
            ~Array();

            int size()const;
            bool empty()const;

            T * data();
            const T * data()const;

            void swap(Array<T,N>& other);
            void fill(const T & value);

            T & front();
            const T & front()const;

            T & back();
            const T & back()const;

            T & at(int index);
            const T & at(int index)const;

            T & operator[](int index);
            const T & operator [](int index)const;

            iterator begin();
            iterator end();

            const_iterator cbegin()const;
            const_iterator cend()const;

            reserve_iterator rbegin();
            reserve_iterator rend();

        private:
            T m_data[N];
    };
    #include "array.inl"
}