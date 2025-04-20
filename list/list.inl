#include "list.h"
using namespace ktg;

template<typename T>
List<T>::List():m_head(nullptr),m_tail(nullptr),m_size(0){

}
template<typename T>
List<T>::~List(){
    clear();
}

template<typename T>
bool List<T>::empty()const{
    return m_size == 0;
}
template<typename T>
int List<T>::size()const{
    return m_size;
}

template<typename T>
T & List<T>::front(){
    if(m_size == 0){
        throw std::out_of_range("list is empty");
    }
    return m_head -> m_value;
}
template<typename T>
const T & List<T>::front()const{
    if(m_size == 0){
        throw std::out_of_range("list is empty");
    }
    return m_head -> m_value;
}

template<typename T>
T & List<T>::back(){
    if(m_size == 0){
        throw std::out_of_range("list is empty");
    }
    return m_tail -> m_value;
}
template<typename T>
const T & List<T>::back()const{
    if(m_size == 0){
        throw std::out_of_range("list is empty");
    }
    return m_tail -> m_value;
}

template<typename T>
void List<T>::clear(){
    while(m_size > 0){
        pop_back();
    }
}

template<typename T>
void List<T>::push_front(const T & value){
    Node * node = new Node(value);
    if(m_size == 0){
        m_head = m_tail = node;
        m_size++;
        return ;
    }

    m_head -> m_prev = node;
    node -> m_next = m_head;
    m_head = node;
    m_size++;
}

template<typename T>
void List<T>::pop_front(){
    if(m_size == 0)return;
    if(m_size == 1){
        delete m_head;
        m_head = m_tail = nullptr;
        m_size--;
        return;
    }

    Node * head = m_head -> m_next;
    head -> m_prev = nullptr;
    delete head;
    m_head = head;
    m_size--;
    return;
}

template<typename T>
void List<T>::display(){
    Node * p = m_head;
    while(p != nullptr){
        std::cout<<p -> m_value<<" ";
        p = p -> m_next;
    }
    std::cout<<std::endl;
}

template<typename T>
void List<T>::push_back(const T & value){
    Node * node = new Node(value);
    if(m_size == 0){
        m_head = m_tail = node;
        m_size++;
        return ;
    }

    m_tail -> m_next = node;
    node -> m_prev = m_tail;
    m_tail = node;
    m_size++;
}

template<typename T>
void List<T>::pop_back(){
    if(m_size == 0)return;
    if(m_size == 1){
        delete m_head;
        m_head = m_tail = nullptr;
        m_size--;
        return;
    }

    Node * tail = m_tail -> m_prev;
    tail -> m_next = nullptr;
    delete m_tail;
    m_tail = tail;
    m_size--;
    return;
}

template<typename T>
typename List<T>::iterator List<T>::begin(){
    return iterator(m_head);
}
template<typename T>
typename List<T>::iterator List<T>::end(){
    if(m_size == 0)return iterator();
    return iterator(m_tail -> m_next);
}

template<typename T>
typename List<T>::const_iterator List<T>::cbegin()const{
    return const_iterator(m_head);
}
template<typename T>
typename List<T>::const_iterator List<T>::cend()const{
    if(m_size == 0)return iterator();
    return const_iterator(m_tail -> m_next);
}

template<typename T>
typename List<T>::reserve_iterator List<T>::rbegin(){
    return reserve_iterator(m_tail);
}
template<typename T>
typename List<T>::reserve_iterator List<T>::rend(){
    if(m_size == 0)return reserve_iterator();
    return reserve_iterator(m_head -> m_prev);
}

template<typename T>
typename List<T>::iterator List<T>::find(const T & value){
    auto node = m_head;
    while(node != nullptr){
        if(node -> m_value == value){
            break;
        }
        node = node -> m_next;
    }
    return iterator(node);
}
template<typename T>
typename List<T>::const_iterator List<T>::find(const T & value)const{
    auto node = m_head;
    while(node != nullptr){
        if(node -> m_value == value){
            break;
        }
        node = node -> m_next;
    }
    return const_iterator(node);
}


template<typename T>
typename List<T>::iterator List<T>::insert(iterator pos, const T & value){
    insert(pos, 1, value);
}


template<typename T>
typename List<T>::iterator List<T>::insert(iterator pos, int n, const T & value){
    if(pos == begin()){
        for(int i = 0; i < n; i++){
            push_front(value);
        }
        return begin();
    }else if(pos == end()){
        auto tail = m_tail;
        for(int i = 0; i < n; i++){
            push_back(value);
        }
        return iterator(tail);
    }

    for(int i = 0; i < n; i++){
        auto node = new Node(value);
        auto prev = pos.m_pointer -> m_prev;
        node -> m_prev = prev;
        prev -> m_next = node;
        node -> m_next = pos.m_pointer;
        pos.m_pointer -> m_prev = node;
        pos.m_pointer = node;
    }
    m_size += n;
    return pos;
}

template<typename T>
typename List<T>::iterator List<T>::erase(iterator pos){
    if(pos == begin()){
        auto node = m_head;
        if(m_size > 1){
            m_head = m_head -> m_next;
            m_head -> m_prev = nullptr;
            delete node;
        }else{
            delete m_head;
            m_head = m_tail = nullptr;
        }
        m_size--;
        return begin();
    }else if(pos == end()){
        return pos;
    }
    auto node = pos.m_pointer;
    if(node -> m_prev != nullptr){
        node -> m_prev -> m_next = node -> m_next;
    }
    if(node -> m_next != nullptr){
        node -> m_next -> m_prev = node -> m_prev;
    }
    auto next = node -> m_next;
    delete node;
    m_size--;
    return iterator(next);

}
template<typename T>
typename List<T>::iterator List<T>::erase(iterator first, iterator last){
    for(auto it = first; it != last; it++){
        erase(it);
    }
    return last;
}

template<typename T>
void List<T>::assign(int n, const T & value){
    clear();
    for(int i = 0; i < n; i++){
        push_back(value);
    }
}
emplate<typename T>
void List<T>::remove(const T & value){
    if(m_size == 0)return;
    while(m_head != nullptr && m_head -> m_value == value){
        auto node = m_head -> m_next;
        node -> m_prev == nullptr;
        delete m_head;
        m_head = node;
        m_size--;
    }
    auto node = m_head;
    while(node != nullptr && node -> m_next){
        if(node -> m_next -> m_value == value){
            auto temp = node -> m_next;
            node -> m_next = temp -> m_next;
            temp -> m_next -> m_prev = node;
            delete temp;
            m_size--;
        }
        node = node -> m_next;
    }
}

template<typename T>
void List<T>::resize(int size){
    if(size < m_size){
        for(int i = 0; i < m_size - size; i++){
            pop_back();
        }
    }else{
        for(int i = 0; i < size - m_size; i++){
            push_back(T());
        }
    }
}

template<typename T>
void List<T>::merge(List<T>&other){
    m_tail -> m_next = other.m_head;
    other.m_head -> m_prev = m_tail;
    m_tail = other.m_tail;
    m_size += other.m_size;

    other.m_head = other.m_tail = nullptr;
    other.m_size = 0;
}

template<typename T>
void List<T>::swap(List<T>&other){
    if(this == &other)return;
    auto head = other.m_head;
    auto tail = other.m_tail;
    int size = other.m_size;

    other.m_head = m_head;
    other.m_tail = m_tail;
    other.m_size = m_size;

    m_head = head;
    m_tail = tail;
    m_size = size;
}

template<typename T>
void List<T>::reserve(){
    if(m_size == 0 || m_size == 1)return;

    auto head = m_head;
    auto tail = m_tail;
    auto node = m_tail;
    while(node != nullptr){
        auto prev = node -> m_prev;
        auto next = node -> m_next;
        node -> m_next = prev;
        node -> m_prev = next;
        node = prev;
    }
    m_head = tail;
    m_tail = head;

}