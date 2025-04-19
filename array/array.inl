
#include "array.h"
using namespace ktg;

template<typename T,size_t N>
Array<T,N>::Array(){
    std::memset(m_data, 0, sizeof(T) * N);
}
template<typename T,size_t N>
Array<T,N>::~Array(){}

template<typename T,size_t N>
int Array<T,N>::size()const{
    return N;
}
template<typename T,size_t N>
bool Array<T,N>::empty()const{
    return N == 0;
}

template<typename T,size_t N>
T * Array<T,N>::data(){
    return m_data;
}
template<typename T,size_t N>
const T * Array<T, N>::data()const{
    return m_data;
}

template<typename T,size_t N>
void Array<T,N>::swap(Array<T,N>& other){
    if(this == &other){
        return;
    }
    for(int i = 0; i < N; i++){
        T temp = other.m_data[i];
        other.m_data[i] = m_data[i];
        m_data[i] = temp;
    }
}
template<typename T,size_t N>
void Array<T,N>::fill(const T & value){
    for(int i = 0; i < N; i++){
        m_data[i] = value;
    }
}

template<typename T,size_t N>
T & Array<T,N>::front(){
    return m_data[0];
}
template<typename T,size_t N>
const T & Array<T,N>::front()const{
    return m_data[0];
}

template<typename T,size_t N>
T & Array<T,N>::back(){
    return m_data[N - 1];
}
template<typename T,size_t N>
const T & Array<T,N>::back()const{
    return m_data[N - 1];
}


template<typename T,size_t N>
T & Array<T,N>::at(int index){
    if(index >= N || index < 0){
        throw std::out_of_range("out of range");
    }
    return m_data[index];
}
template<typename T,size_t N>
const T & Array<T,N>::at(int index)const{
    if(index >= N || index < 0){
        throw std::out_of_range("out of range");
    }
    return m_data[index];
}

template<typename T,size_t N>
T & Array<T,N>::operator[](int index){
    return m_data[index];
}
template<typename T,size_t N>
const T & Array<T,N>::operator [](int index)const{
    return m_data[index];
}

template<typename T,size_t N>
typename Array<T,N>::iterator  Array<T,N>::begin(){
    return iterator(m_data);
}
template<typename T,size_t N>
typename Array<T,N>::iterator Array<T,N>::end(){
    return iterator(m_data + N);
}   

template<typename T,size_t N>
typename Array<T,N>::const_iterator Array<T,N>::cbegin() const{
    return const_iterator(m_data);
}
template<typename T,size_t N>
typename Array<T,N>::const_iterator Array<T,N>::cend() const{
    return const_iterator(m_data + N);
}

template<typename T,size_t N>
typename Array<T,N>::reserve_iterator Array<T,N>::rbegin(){
    return reserve_iterator(m_data + N - 1);
}
template<typename T,size_t N>
typename Array<T,N>::reserve_iterator Array<T,N>::rend(){
    return reserve_iterator(m_data - 1);
}