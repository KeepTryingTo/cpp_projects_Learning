templeta<class T,class Alloc=alloc>
class vector {
    public:
    typedef T value_type;
    typedef value_type *pointer;
    typedef value_type &reference;
    typedef value_type *iterator;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type
    //嵌套类型定义，也可以是关联类型定义
    protected:
    typedef simple_alloc <value_type, Alloc> data_alloctor
    //空间配置器（分配器）
    iterator start;
    iterator finish;
    iterator end_of_storage;
    //这3个就是vector⾥的数据，所以⼀个vector就是包含3个指针12byte,下⾯有图介绍
    void insert_aux(iterator position, const T &x);
    //这个就是vector的⾃动扩充函数，在下⾯章节我会拿出来分析
    void deallocate() {
        if (start)
        data_allocator::deallocate(start, end_of_storage);
    }
    //析构函数的部分实现函数
    void fill_initialize(size_type n, const T &value) {
    start = allocate_and_fill(n, value);
    finish = start + n;
    end_of_storage = finish;
        }
    //构造函数的具体实现
    public:
    iterator begin() { return start; };
    iterator end() { return finish; };
    size_type size() const { return size_type(end() - begin()); };
    size_type capacity() const { return size_type(end_of_storage - begin()); }
    bool empty() const { return begin() == end(); }
    reference operator[](size_type n) { return *(begin() + n); };
    //重载[]说明vector⽀持随机访问
    vector() : start(0), end(0), end_of_storage(0) {};
    vector(size_type n, const T &value)(fill_initialize(n, value););
    vector(long n, const T &value)(fill_initialize(n, value););
    vector(int n, const T &value)(fill_initialize(n, value););
    explicit vector(size_type n) { fill_initialize(n, T()); };
    //重载构造函数
    ~vector() {
    destory(start, finish);//全局函数，析构对象
    deallocate();//成员函数，释放空间
        }
    //接下来就是⼀些功能函数
    reference front() { return *begin(); };
    reference back() { return *(end() - 1); };
    void push_back(const T &x) {
    if (finsih != end_of_storage) {
    construct(finish, x);
    ++finish;
            } 
    else insert_aux(end(), x);
    //先扩充在添加
        }
    void pop_back() {--finish;
    destory(finish);
        }
    iterator erase(iterator position) {
    if (position + 1 != end())
    copy(position + 1, finish, position);--finish;
    destory(finish);
    return position;
        }
    void resize(size_type new_size, const T &x) {
    if (new_size() < size())
    erase(begin() + new_size, end());
    else
    insert(end(), new_size - size(),x);
        }
    void resize()(size_type new_size) { resize(new_size, T()); }
    void clear() { erase(begin(), end()); }
    protected:
    //配置空间并填满内容
    iterator allocate_and_fill(size_type n, const T &x) {
    iterator result = data_allocator::allocate(n);
    uninitialized_fill_n(result, n, x);//全局函数
        }
    }
    template <class T, class Alloc = alloc>
    class vector {
    public:
        typedef T value_type;
        typedef value_type* iterator; //vector的迭代器是原⽣指针
// ...
 };