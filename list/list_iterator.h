#pragma once

#include "node.h"
using namespace ktg;

namespace ktg {
    template <typename T>
    class List;
    template<typename T>
    class ListIterator{
        public:
            friend class List<T>;
            typedef ListIterator<T>iterator;
            typedef ListNode<T>Node;
        public:
            ListIterator(){};
            ListIterator(Node * pointer):m_pointer(pointer){}
            ~ListIterator(){};

            bool operator == (const iterator & other){
                return m_pointer == other.m_pointer;
            }
            bool operator != (const iterator & other){
                return m_pointer != other.m_pointer;
            }

            iterator & operator = (const iterator & other){
                if(this == &other){
                    return *this;
                }
                m_pointer = other.m_pointer;
                return *this;
            }

            //前缀++
            iterator & operator ++(){
                m_pointer = m_pointer -> m_next;
                return *this;
            }
            iterator operator ++(int){
                iterator it = *this;
                ++(*this);
                return it;
            }

            iterator operator + (int n){
                iterator it = *this;
                for(int i = 0; i < n; i++){
                    ++it;
                }
                return it;
            }

            iterator & operator += (int n){
                for(int i = 0; i < n; i++){
                    ++(*this);
                }
                return *this;
            }
            
            iterator & operator --(){
                m_pointer = m_pointer -> m_prev;
                return *this;
            }
            iterator operator --(int){
                iterator it = *this;
                --(*this);
                return it;
            }
            iterator operator - (int n){
                iterator it = *this;
                for(int i = 0; i < n; i++){
                    --it;
                }
                return it;
            }

            iterator & operator -= (int n){
                for(int i = 0; i < n; i++){
                    --(*this);
                }
                return *this;
            }

            T & operator * (){
                return m_pointer -> m_value;
            }
            T * operator -> (){
                return &(m_pointer -> m_value);
            }

        
        private:
            Node * m_pointer;
    };
}