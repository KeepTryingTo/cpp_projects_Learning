/*-
 * Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */


/*-
 * A template class that implements a simple stack structure.
 * This demostrates how to use alloctor_traits (specifically with MemoryPool)
 * 
 * allocate      分配一个对象所需的内存空间
   deallocate    释放一个对象的内存（归还给内存池，不是给操作系统）
   construct     在已申请的内存空间上构造对象
   destroy       析构对象
   newElement    从内存池申请一个对象所需空间，并调用对象的构造函数
   deleteElement 析构对象，将内存空间归还给内存池
   allocateBlock 从操作系统申请一整块内存放入内存池
 */

#ifndef STACK_ALLOC_H
#define STACK_ALLOC_H

#include <memory>

template <typename T>
struct StackNode_
{
  T data;
  StackNode_* prev;
};

/** T is the object to store in the stack, Alloc is the allocator to use */
template <class T, class Alloc = std::allocator<T> >
class StackAlloc
{
  public:
  //定义别名
    typedef StackNode_<T> Node;
    //为分配器类型定义一个别名 allocator，用于分配 Node 类型的内存。
    /*
      rebind 是分配器的一个成员模板，用于将分配器从分配 T 类型的内存重新绑定为分配 Node 类型的内存。
      typename 用于告诉编译器 Alloc::template rebind<Node>::other 是一个类型。
      other 是 rebind 的结果类型，表示一个新的分配器类型，专门用于分配 Node 类型的内存。
    */
    typedef typename Alloc::template rebind<Node>::other allocator;

    /** Default constructor */
    StackAlloc() {head_ = 0; }
    /** Default destructor */
    ~StackAlloc() { clear(); }

    /** Returns true if the stack is empty */
    bool empty() {return (head_ == 0);}

    /** Deallocate all elements and empty the stack 清空栈*/
    void clear() {
      Node* curr = head_;
      while (curr != 0)
      {
        Node* tmp = curr->prev;
        //析构当前对象
        allocator_.destroy(curr);
        // 释放一个对象的内存（归还给内存池，不是给操作系统）
        allocator_.deallocate(curr, 1);
        curr = tmp;
      }
      head_ = 0;
    }

    /** Put an element on the top of the stack */
    void push(T element) {
      // 分配一个对象所需的内存空间
      Node* newNode = allocator_.allocate(1);
      //在已申请的内存空间上构造对象
      allocator_.construct(newNode, Node());
      //赋值并且移动指针位置
      newNode->data = element;
      newNode->prev = head_;
      head_ = newNode;
    }

    /** Remove and return the topmost element on the stack 弹出栈顶*/
    T pop() {
      //获取栈顶元素
      T result = head_->data;
      //记录当前栈顶节点
      Node* tmp = head_->prev;
      //析构和释放对象（归还给内存池）
      allocator_.destroy(head_);
      allocator_.deallocate(head_, 1);
      head_ = tmp;
      return result;
    }

    /** Return the topmost element 返回栈顶*/
    T top() { return (head_->data); }

  private:
    allocator allocator_;
    Node* head_;
};

#endif // STACK_ALLOC_H
