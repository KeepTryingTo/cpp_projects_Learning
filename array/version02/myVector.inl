#include "myVector.h"
using namespace ktg;

template<typename T>
myVector<T>::myVector():m_data(nullptr),m_size(0), m_capacity(0){}

template<typename T>
myVector<T>::~myVector(){
    if(m_data != nullptr){
        delete []m_data;
    }
    m_size = 0;
    m_capacity = 0;
}

template<typename T>
T & myVector<T>::at(int index){
    if(index < 0 || index >= m_size){
        throw std::out_of_range("out of range");
    }

    return m_data[index];
}

template<typename T>
const T & myVector<T>::at(int index) const{
    if(index < 0 || index >= m_size){
        throw std::out_of_range("out of range");
    }

    return m_data[index];
}

template<typename T>
T & myVector<T>::front(){
    if(m_size == 0){
        throw std::out_of_range("size element is zero");
    }
    return m_data[0];
}

template<typename T>
const T & myVector<T>::front()const{
    if(m_size == 0){
        throw std::out_of_range("size element is zero");
    }
    return m_data[0];
}

template<typename T>
T & myVector<T>::back(){
    if(m_size == 0){
        throw std::out_of_range("size element is zero");
    }
    return m_data[m_size - 1];
}

template<typename T>
const T & myVector<T>::back()const{
    if(m_size == 0){
        throw std::out_of_range("size element is zero");
    }
    return m_data[m_size - 1];
}

template<typename T>
T * myVector<T>::data(){
    return m_data;
}
template<typename T>
const T * myVector<T>::data()const{
    return m_data;
}

template<typename T>
T & myVector<T>::operator[] (int index){
    if(index < 0 || index >= m_size){
        throw std::out_of_range("out of range");
    }

    return m_data[index];
}
template<typename T>
const T & myVector<T>::operator[] (int index)const{
    if(index < 0 || index >= m_size){
        throw std::out_of_range("out of range");
    }

    return m_data[index];
}
template<typename T>
void myVector<T>::push_back(const T & value){
    if(m_size < m_capacity){
        m_data[m_size] = value;
        m_size ++;
        return;
    }
    if(m_capacity == 0){
        m_capacity = 1;
    }else{
        m_capacity *= 2;
    }

    T * data = new T[m_capacity];
    for(int i = 0; i < m_size; i++){
        data[i] = m_data[i];
    }
    if(m_data){
        delete []m_data;
        m_data = nullptr;
    }
    m_data = data;
    m_data[m_size] = value;
    m_size++;
    return;
}

template<typename T>
void myVector<T>::pop_back(){
    if(m_size > 0){
        m_size--;
    }
}


template<typename T>
void myVector<T>::display() const {
    std::cout<<"size = "<<m_size<<" capacity = "<<m_capacity<<std::endl;
    for(int i = 0; i < m_size; i++){
        std::cout<<m_data[i]<<" ";
    }
    std::cout<<std::endl;
}
template<typename T>
void myVector<T>::assign(int n, const T & val){
    if(m_capacity >= n){
        for(int i = 0; i < n; i++){
            m_data[i] = val;
        }
        m_size = n;
        return;
    }
    if(m_data != nullptr){
        delete []m_data;
        m_data = nullptr;
        m_size = 0;
        m_capacity = 0;
    }
    while(m_capacity < n){
        if(m_capacity == 0){
            m_capacity = 1;
        }else{
            m_capacity *= 2;
        }
    }
    m_data = new T[m_capacity];
    for(int i = 0; i < n; i++){
        m_data[i] = val;
    }
    m_size = n;
}
template<typename T>
void myVector<T>::swap(myVector<T>&other){
    if(this == &other){
        return;
    }
    T * data = other.m_data;
    int size = other.m_size;
    int capacity = other.m_capacity;

    other.m_data = m_data;
    other.m_capacity = m_capacity;
    other.m_size = m_size;

    m_data = data;
    m_size = size;
    m_capacity = capacity;
}
template<typename T>
bool myVector<T>::empty()const{
    return m_size == 0;
}
template<typename T>
int myVector<T>::size()const{
    return m_size;
}
template<typename T>
int myVector<T>::capacity()const{
    return m_capacity;
}

template<typename T>
void myVector<T>::resize(int n){
    resize(n, T());
}

template<typename T>
void myVector<T>::resize(int n, const T & val){
    if(n < m_size){
        m_size = n;
        return;
    }
    if(n < m_capacity){
        for(int i = m_size; i < n; i++){
            m_data[i] = val;
        }
        m_size = n;
        return;
    }
    
    while(m_capacity < n){
        if(m_capacity == 0){
            m_capacity = 1;
        }else{
            m_capacity *= 2;
        }
    }

    T * data = new T[m_capacity];
    for(int i = 0; i < m_size; i++){
        data[i] = m_data[i];
    }
    for(int i = m_size; i < n; i++){
        data[i] = val;
    }
    if(m_data != nullptr){
        delete []m_data;
        m_data = nullptr;
    }
    m_size = n;
    m_data = data;
}

template<typename T>
void myVector<T>::reserve(int n){
    if(m_capacity > n){
        return ;
    }
    while(m_capacity < n){
        if(m_capacity == 0){
            m_capacity = 1;
        }else{
            m_capacity *= 2;
        }
    }
    T * data = new T[m_capacity];
    for(int i = 0; i < m_size; i++){
        data[i] = m_data[i];
    }
    if(m_data != nullptr){
        delete []m_data;
        m_data = nullptr;
    }
    m_data = data;
}

template<typename T>
typename myVector<T>::iterator myVector<T>::begin(){
    return iterator(m_data);
}

template<typename T>
typename myVector<T>::iterator  myVector<T>::end(){
    return iterator(m_data + m_size);
}

template<typename T>
typename myVector<T>::const_iterator myVector<T>::cbegin()const{
    return const_iterator(m_data);
}

template<typename T>
typename myVector<T>::const_iterator myVector<T>::cend()const{
    return const_iterator(m_data + m_size);
}

template<typename T>
typename myVector<T>::reserve_iterator myVector<T>::rbegin(){
    return reserve_iterator(m_data + m_size - 1);
}

template<typename T>
typename myVector<T>::reserve_iterator myVector<T>::rend(){
    return reserve_iterator(m_data - 1);
}

template<typename T>
typename myVector<T>::iterator myVector<T>::insert(iterator pos, const T & value){
    insert(pos, 1, value);
}

template<typename T>
typename myVector<T>::iterator myVector<T>::insert(iterator pos, int n, const T & value){
    int size = pos - begin();
    if(m_size + n <= m_capacity){
        for(int i = m_size; i > size; i--){
            m_data[i + n - 1] = m_data[i - 1];
        }
        for(int i = 0; i < n; i++){
            m_data[i + size] = value;
        }
        m_size += n;
        return iterator(m_data + size);
    }

    while(m_size + n > m_capacity){
        if(m_capacity == 0){
            m_capacity = 1;
        }else{
            m_capacity *= 2;
        }
    }

    T * data = new T[m_capacity];
    for(int i = 0; i < size; i++){
        data[i] = m_data[i];
    }
    for(int i = 0; i < n; i++){
        data[i + size] = value;
    }
    for(int i = size; i < m_size; i++){
        data[i + n] = m_data[i];
    }

    if(m_data != nullptr){
        delete []m_data;
        m_data = nullptr;
    }
    m_data = data;
    m_size += n;
    return iterator(m_data + size);
}

template<typename T>
typename myVector<T>::iterator myVector<T>::erase(iterator pos){
    if(pos == end()){
        throw std::out_of_range("out of range");
    }
    if(end() - pos == 1){
        m_size -= 1;
        return end();
    }

    int size = pos - begin();
    for(int i = size; i < m_size - 1; i++){
        m_data[i] = m_data[i + 1];
    }
    m_size -= 1;
    return pos;
}


template<typename T>
typename myVector<T>::iterator myVector<T>::erase(iterator first, iterator last){
    int f = first - begin();
    int l = last - begin();
    int n = last - first;
    if(is_basic_type()){
        std::memmove(m_data + f, m_data + l , (m_size - l) * sizeof(T));
    }else{
        for(int i = 0; i < m_size - l; i++){
            m_data[f + i] = m_data[l + i];
        }
    }
    m_size -= n;
    return first;
}

template<typename T>
void myVector<T>::clear(){
    m_size = 0;
}

template<typename T>
bool myVector<T>::is_basic_type(){
    if(std::is_pointer<T>::value){
        return true;
    }
    return  (typeid(T) == typeid(bool)) || 
            (typeid(T) == typeid(char)) ||
            (typeid(T) == typeid(unsigned char))||
            (typeid(T) == typeid(short)) ||
            (typeid(T) == typeid(unsigned short))||
            (typeid(T) == typeid(int)) ||
            (typeid(T) == typeid(unsigned int)) ||
            (typeid(T) == typeid(long)) ||
            (typeid(T) == typeid(unsigned long)) ||
            (typeid(T) == typeid(float)) ||
            (typeid(T) == typeid(double));
}