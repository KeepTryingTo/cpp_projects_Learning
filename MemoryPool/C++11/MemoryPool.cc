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
 
 #ifndef MEMORY_BLOCK_TCC
 #define MEMORY_BLOCK_TCC

 #include "MemoryPool.h"

 /*
  allocate         分配一个对象所需的内存空间
  deallocate       释放一个对象的内存（归还给内存池，不是给操作系统）
  construct        在已申请的内存空间上构造对象
  destroy          析构对象
  newElement       从内存池申请一个对象所需空间，并调用对象的构造函数
  deleteElement    析构对象，将内存空间归还给内存池
  allocateBlock    从操作系统申请一整块内存放入内存池

  注意：allocate和newElement的区别，allocate分配的内存放入内存池中管理，如果需要申请空间，newElement从内存池中分配一个对象
 */
 
 // 计算对齐所需补的空间
 template <typename T, size_t BlockSize>
 inline typename MemoryPool<T, BlockSize>::size_type 
 MemoryPool<T, BlockSize>::padPointer(data_pointer_ p, size_type align) const throw(){
  //reinterpret_cast 是 C++ 中的一种类型转换运算符，用于低级别的类型转换。
  //它可以将任意指针类型转换为另一种指针类型，甚至可以将指针转换为整数类型，或者将整数类型转换为指针类型。
   size_t result = reinterpret_cast<size_t>(p);
   std::cout<<"result = "<<result<<"  sizeof(result) = "<<sizeof(result)<<std::endl;
   //计算当前需要多少空间来对齐（计算当前指针 p 需要 ​​填充多少字节​​ 才能对齐到 align 边界）
   return ((align - result) % align);
 }
 
 /* 构造函数，所有成员初始化 */
 template <typename T, size_t BlockSize>
 MemoryPool<T, BlockSize>::MemoryPool() throw() {
   this -> currentBlock_ = 0;
   this -> currentSlot_ = 0;
   this -> lastSlot_ = 0;
   this -> freeSlots_ = 0;
 }
 
 /* 复制构造函数,调用 MemoryPool 初始化*/
 template <typename T, size_t BlockSize>
 MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool& memoryPool)throw(){
   MemoryPool();
 }
 
 /* 复制构造函数,调用 MemoryPool 初始化*/
 template <typename T, size_t BlockSize>
 template<class U>
 MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool<U>& memoryPool)throw(){
   MemoryPool();
 }
 
 /* 析构函数，把内存池中所有 block delete 掉 */
 template <typename T, size_t BlockSize>
 MemoryPool<T, BlockSize>::~MemoryPool()throw(){
  //内存块链表头指针
   slot_pointer_ curr = currentBlock_;
   //如果链表不为空就进行释放
   while (curr != 0) {
     slot_pointer_ prev = curr->next;
     //将当前指定的内存块给释放掉， 转化为 void 指针，是因为 void 类型不需要调用析构函数,只释放空间
     operator delete(reinterpret_cast<void*>(curr));
     //继续释放下一个节点的空间
     curr = prev;
   }
 }
 
 /* 返回地址 */
 template <typename T, size_t BlockSize>
 inline typename MemoryPool<T, BlockSize>::pointer //返回T类型的指针
 MemoryPool<T, BlockSize>::address(reference x)const throw(){
  //返回变量地址
   return &x;
 }
 
 /* 返回地址的 const 重载*/
 template <typename T, size_t BlockSize>
 inline typename MemoryPool<T, BlockSize>::const_pointer //const 指针类型
 MemoryPool<T, BlockSize>::address(const_reference x)const throw(){
   return &x;
 }
 
 // 申请一块空闲的 block 放进内存池，但是放入内存池，其实使用的是更小的slot来进行管理，也就是需要对申请的block继续划分为多个slot
 template <typename T, size_t BlockSize>
 void
 MemoryPool<T, BlockSize>::allocateBlock()
 {
   // Allocate space for the new block and store a pointer to the previous one
   // operator new 申请对应大小内存，返回 void* 指针 // data_pointer_ 为char* 指针，主要用于指向内存首地址
   std::cout<<"BlockSize = "<<BlockSize<<std::endl;
   //申请空间并转换为char*类型
   data_pointer_ newBlock = reinterpret_cast<data_pointer_>(operator new(BlockSize));

   // 原来的 block 链头接到 newblock 将当前块（currentBlock_）的指针链接到新分配的块（newBlock）中，
   //通常用于形成一个链表结构，以便管理多个内存块。
   reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;

   // 新的 currentblock_ 更新 currentBlock_ 指向新分配的块 (newBlock)，这样后续的内存分配将从这个新块开始。
   //其实就是指针移动 
   currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);

   // 对块体进行填充，以满足元素的对齐要求 body 指向 newBlock 的起始位置加上 slot_pointer_ 的大小
   data_pointer_ body = newBlock + sizeof(slot_pointer_);
   //由于body是指针类型，sizeof计算的是起始地址大小8
   std::cout<<"newBlock size = "<<sizeof(newBlock)<<" body size = "<<sizeof(body)<<std::endl;
   std::cout<<"slot pointer size = "<<sizeof(slot_pointer_)<<" slot type size = "<<sizeof(slot_type_)<<std::endl;

   // 计算为了对齐应该空出多少位置 调用 padPointer 函数来计算 body 需要填充多少字节，以满足 slot_type_ 的对齐要求。
   //对齐是为了提高内存访问的效率，防止因不对齐而导致的性能问题。
   size_type bodyPadding = padPointer(body, sizeof(slot_type_));
   std::cout<<"bodyPadding = "<<bodyPadding<<std::endl;

   // currentslot_ 为该 block 开始的地方加上 bodypadding 个 char* 空间
   currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);//指针内存对齐
   // 计算最后一个能放置 slot_type_ 的位置
   //当前新申请的内存块newBlock，而slot是内存块更为细分的划分方式，为的是减少内存碎片
   //这里+BlockSize是为将lastSlot_指针移动指向最后一个slot
   lastSlot_ = reinterpret_cast<slot_pointer_>
               (newBlock + BlockSize - sizeof(slot_type_) + 1);
 }
 
 // 返回指向分配新元素所需内存的指针，默认每一次申请一个BlockSize
 template <typename T, size_t BlockSize>
 inline typename MemoryPool<T, BlockSize>::pointer
 MemoryPool<T, BlockSize>::allocate(size_type, const_pointer)
 {
   // 如果 freeSlots_ 非空，就在 freeSlots_ 中取内存
   /*
   空闲链表初始状态：
      freeSlots_ → [Slot_1] → [Slot_2] → [Slot_3] → nullptr
                      
                   reinterpret_cast

                  | next   | → Slot_2
                  | element| （未使用）
   */
   if (freeSlots_ != 0) {
     pointer result = reinterpret_cast<pointer>(freeSlots_);
     // 更新 freeSlots_
     freeSlots_ = freeSlots_->next;
     return result;
   }
   else {
    //判断当前指向的头部slot是否等于指向尾部的slot，如果是，就需要重新申请block
     if (currentSlot_ >= lastSlot_)
       // 之前申请的内存用完了，分配新的 block
       allocateBlock();
       // 从分配的block中划分出一个slot
     return reinterpret_cast<pointer>(currentSlot_++);
   }
 }
 
 // 将元素内存归还给 free 内存链表保存（也就是归还给内存池）
 template <typename T, size_t BlockSize>
 inline void
 MemoryPool<T, BlockSize>::deallocate(pointer p, size_type)
 {
   if (p != 0) {
     // 转换成 slot_pointer_ 指针，next 指向 freeSlots_ 链表
     reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
     // 新的 freeSlots_ 头为 p
     freeSlots_ = reinterpret_cast<slot_pointer_>(p);
   }
 }
 
 // 计算可达到的最大元素上限数
 template <typename T, size_t BlockSize>
 inline typename MemoryPool<T, BlockSize>::size_type
 MemoryPool<T, BlockSize>::max_size()const throw(){
  //在 C++ 中，-1 是一个有符号整数。但是，当它被赋值给一个无符号整数时，会发生 隐式类型转换。
  //当 -1 被赋值给无符号整数时，它会被解释为无符号整数的最大值。其中BlockSize为无符号整数
   size_type maxBlocks = -1 / BlockSize;//最多为4503599627380495
   std::cout<<"maxBlock = "<<maxBlocks<<std::endl;
   //因为data_pointer_是char*和slot_type_采用的是联合体结构，
   //代表当前的block可以最多划分为多少个slot以及乘以block的数量
   return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;
 }
 
 // 在已分配内存上构造对象
 template <typename T, size_t BlockSize>
 inline void
 MemoryPool<T, BlockSize>::construct(pointer p, const_reference val)
 {
   // placement new 用法，在已有内存上构造对象，调用 T 的复制构造函数，new的类型为val
   //https://blog.csdn.net/justaipanda/article/details/7790355
   //在指定的内存地址p申请新的内存空间
   new (p) value_type (val);
 }
 
 // 销毁对象
 template <typename T, size_t BlockSize>
 inline void
 MemoryPool<T, BlockSize>::destroy(pointer p)
 {
   // placement new 中需要手动调用元素 T 的析构函数
   p->~value_type();
 }
 
 // 创建新元素
 template <typename T, size_t BlockSize>
 inline typename MemoryPool<T, BlockSize>::pointer
 MemoryPool<T, BlockSize>::newElement(const_reference val)
 {
   // 申请内存
   pointer result = allocate();
   // 在内存上构造对象
   construct(result, val);
   return result;
 }
 
 // 删除元素
 template <typename T, size_t BlockSize>
 inline void
 MemoryPool<T, BlockSize>::deleteElement(pointer p)
 {
   if (p != 0) {
     // placement new 中需要手动调用元素 T 的析构函数
     p->~value_type();
     // 归还内存给内存池
     deallocate(p);
   }
 }
 
 #endif // MEMORY_BLOCK_TCC