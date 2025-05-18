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

 #ifndef MEMORY_POOL_H
 #define MEMORY_POOL_H
 
 #include <limits.h>
 #include <stddef.h>
 
 //https://www.cnblogs.com/wpcockroach/archive/2012/05/10/2493564.html
/*
  newElement()	分配内存 + 构造对象（等价于 placement new）
  deleteElement()	析构对象 + 回收内存
  construct()	在已分配内存上构造对象
  destroy()	析构对象但不释放内存
  max_size()	返回理论可分配的最大元素数（依赖 BlockSize）
*/
 template <typename T, size_t BlockSize = 4096>
 class MemoryPool
 {
   public:
     /* Member types 定义别名*/
     typedef T               value_type;       // T 的 value 类型
     typedef T*              pointer;          // T 的 指针类型
     typedef T&              reference;        // T 的引用类型
     typedef const T*        const_pointer;    // T 的 const 指针类型
     typedef const T&        const_reference;  // T 的 const 引用类型
     typedef size_t          size_type;        // size_t 类型
     typedef ptrdiff_t       difference_type;  // 指针减法结果类型
 
     template <typename U> 
     struct rebind {
       typedef MemoryPool<U> other;
     };
 
     /* Member functions */
     /* 构造函数 */
     MemoryPool() throw();//默认构造函数
     MemoryPool(const MemoryPool& memoryPool) throw();//拷贝构造函数
     template <class U> MemoryPool(const MemoryPool<U>& memoryPool) throw();
 
     /* 析构函数 */
     ~MemoryPool() throw();
 
     /* 元素取址 */
     pointer address(reference x) const throw();
     const_pointer address(const_reference x) const throw();
 
     // Can only allocate one object at a time. n and hint are ignored
     // 分配和收回一个元素的内存空间
     pointer allocate(size_type n = 1, const_pointer hint = 0);
     void deallocate(pointer p, size_type n = 1);
 
     // 可达到的最多元素数
     size_type max_size() const throw();
 
     // 基于内存池的元素构造和析构
     void construct(pointer p, const_reference val);
     void destroy(pointer p);
 
     // 自带申请内存和释放内存的构造和析构
     pointer newElement(const_reference val);
     void deleteElement(pointer p);
 
   private:
     // union 结构体,用于存放元素或 next 指针
     /*
     联合体（union）​​ 的巧妙利用：
        分配时作为 element 存储用户数据
        释放时作为 next 指针加入空闲链表
     */
     union Slot_ {
       //类型为T
       value_type element;
       Slot_* next;
     };

     /*
     
     Slot_ 在创建对象的时候存放对象的值，当这个对象被释放时这块内存作为一个 Slot_* 指针放入 free 的链表。
          所以 Slot_ 既可以用来存放对象，又可以用来构造链表。
     工作原理
          内存池是一个一个的 block 以链表的形式连接起来，每一个 block 是一块大的内存，当内存池的内存不足的时候，
          就会向操作系统申请新的 block 加入链表。还有一个 freeSlots_ 的链表，链表里面的每一项都是对象被释放后归还给内存池的空间，
          内存池刚创建时 freeSlots_ 是空的，之后随着用户创建对象，再将对象释放掉，这时候要把内存归还给内存池，怎么归还呢？
          就是把指向这个对象的内存的指针加到 freeSlots_ 链表的前面（前插）。
          用户在创建对象的时候，先检查 freeSlots_ 是否为空，不为空的时候直接取出一项作为分配出的空间。
          否则就在当前 block 内取出一个 Slot_ 大小的内存分配出去，如果 block 里面的内存已经使用完了呢？就向操作系统申请一个新的 block。
          内存池工作期间的内存只会增长，不释放给操作系统。直到内存池销毁的时候，才把所有的 block delete 掉。
     */ 
     //定义别名
     typedef char* data_pointer_;  // char* 指针，主要用于指向内存首地址
     typedef Slot_ slot_type_;     // Slot_ 值类型
     typedef Slot_* slot_pointer_; // Slot_* 指针类型
 
     slot_pointer_ currentBlock_;  // 内存块链表的头指针
     slot_pointer_ currentSlot_;   // 元素链表的头指针
     slot_pointer_ lastSlot_;      // 可存放元素的最后指针
     slot_pointer_ freeSlots_;     // 元素构造后释放掉的内存链表头指针
 
     size_type padPointer(data_pointer_ p, size_type align) const throw();  // 计算对齐所需空间
     void allocateBlock();  // 申请内存块放进内存池
    /*
     static_assert(BlockSize >= 2 * sizeof(slot_type_), "BlockSize too small.");
     */
 };
 
 #include "MemoryPool.cc"
 
 #endif // MEMORY_POOL_H