#pragma once 

#include <stdexcept>
#include "node.h"
#include "list_iterator.h"
#include "list_const_iterator.h"
#include "list_reserve_iterator.h"
using namespace ktg;

namespace ktg {
    template<typename T>
    class List {
        public:
            typedef ListNode<T>Node;
            typedef ListIterator<T>iterator;
            typedef ListConstIterator<T>const_iterator;
            typedef ListReserveIterator<T>reserve_iterator;
        public:
            List();
            ~List();

            bool empty()const;
            int size()const;

            T & front();
            const T & front()const;

            T & back();
            const T & back()const;

            void clear();
            void assign(int n, const T & value);
            void remove(const T & value);
            void resize(int size);
            void merge(List<T>&other);
            void swap(List<T>&other);
            void reserve();

            void push_front(const T & value);
            void pop_front();

            void push_back(const T & value);
            void pop_back();

            void display();

            iterator begin();
            iterator end();
            iterator find(const T & value);
            iterator insert(iterator pos, const T & value);
            iterator insert(iterator pos, int n, const T & value);
            iterator erase(iterator pos);
            iterator erase(iterator first, iterator last);

            const_iterator cbegin()const;
            const_iterator cend()const;
            const_iterator find(const T & value)const;

            reserve_iterator rbegin();
            reserve_iterator rend();

        private:
            Node * m_head;
            Node * m_tail;
            int m_size;
    };

    #include "list.inl"
}