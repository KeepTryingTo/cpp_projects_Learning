#include <algorithm>    // std::copy, std::move, std::fill
#include <cassert>      // assert
#include <initializer_list>
#include <memory>       // std::allocator, std::allocator_traits
#include <stdexcept>    // std::out_of_range
#include <utility>      // std::exchange, std::move

template <typename T, typename Allocator = std::allocator<T>>
class vector {
public:
    // 类型别名（与标准库一致）
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // 默认构造函数
    explicit vector(const Allocator& alloc = Allocator()) noexcept
        : alloc_(alloc), data_(nullptr), size_(0), capacity_(0) {}

    // 构造函数：指定初始大小和默认值
    explicit vector(size_type count, const T& value = T(),
                    const Allocator& alloc = Allocator())
        : alloc_(alloc), data_(nullptr), size_(count), capacity_(count) {
        if (count > 0) {
            data_ = allocate(count);
            std::uninitialized_fill_n(data_, size_, value);
        }
    }

    // 构造函数：通过初始化列表
    vector(std::initializer_list<T> init, const Allocator& alloc = Allocator())
        : alloc_(alloc), data_(nullptr), size_(init.size()), capacity_(init.size()) {
        if (size_ > 0) {
            data_ = allocate(size_);
            std::uninitialized_copy(init.begin(), init.end(), data_);
        }
    }

    // 析构函数
    ~vector() {
        clear();
        deallocate(data_, capacity_);
    }

    // 拷贝构造函数
    vector(const vector& other)
        : alloc_(std::allocator_traits<Allocator>::select_on_container_copy_construction(
              other.alloc_)),
          data_(nullptr), size_(other.size_), capacity_(other.size_) {
        if (size_ > 0) {
            data_ = allocate(size_);
            std::uninitialized_copy(other.data_, other.data_ + size_, data_);
        }
    }

    // 移动构造函数
    vector(vector&& other) noexcept
        : alloc_(std::move(other.alloc_)),
          data_(std::exchange(other.data_, nullptr)),
          size_(std::exchange(other.size_, 0)),
          capacity_(std::exchange(other.capacity_, 0)) {}

    // 拷贝赋值运算符
    vector& operator=(const vector& other) {
        if (this != &other) {
            // 处理分配器（根据 propagate_on_container_copy_assignment）
            using alloc_traits = std::allocator_traits<Allocator>;
            if constexpr (alloc_traits::propagate_on_container_copy_assignment::value) {
                if (alloc_ != other.alloc_) {
                    clear();
                    deallocate(data_, capacity_);
                    data_ = nullptr;
                    capacity_ = 0;
                }
                alloc_ = other.alloc_;
            }

            // 重新分配内存并拷贝元素
            if (other.size_ > capacity_) {
                clear();
                deallocate(data_, capacity_);
                data_ = allocate(other.size_);
                capacity_ = other.size_;
            }
            size_ = other.size_;
            std::uninitialized_copy(other.data_, other.data_ + size_, data_);
        }
        return *this;
    }

    // 移动赋值运算符
    vector& operator=(vector&& other) noexcept {
        if (this != &other) {
            // 处理分配器（根据 propagate_on_container_move_assignment）
            using alloc_traits = std::allocator_traits<Allocator>;
            if constexpr (alloc_traits::propagate_on_container_move_assignment::value) {
                alloc_ = std::move(other.alloc_);
            }

            // 释放当前资源并接管 other 的资源
            clear();
            deallocate(data_, capacity_);
            data_ = std::exchange(other.data_, nullptr);
            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
        }
        return *this;
    }

    // 元素访问
    reference operator[](size_type pos) {
        assert(pos < size_);
        return data_[pos];
    }

    const_reference operator[](size_type pos) const {
        assert(pos < size_);
        return data_[pos];
    }

    reference at(size_type pos) {
        if (pos >= size_) throw std::out_of_range("vector::at");
        return data_[pos];
    }

    const_reference at(size_type pos) const {
        if (pos >= size_) throw std::out_of_range("vector::at");
        return data_[pos];
    }

    reference front() { return data_[0]; }
    const_reference front() const { return data_[0]; }
    reference back() { return data_[size_ - 1]; }
    const_reference back() const { return data_[size_ - 1]; }
    pointer data() noexcept { return data_; }
    const_pointer data() const noexcept { return data_; }

    // 迭代器
    iterator begin() noexcept { return data_; }
    const_iterator begin() const noexcept { return data_; }
    iterator end() noexcept { return data_ + size_; }
    const_iterator end() const noexcept { return data_ + size_; }
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

    // 容量
    bool empty() const noexcept { return size_ == 0; }
    size_type size() const noexcept { return size_; }
    size_type capacity() const noexcept { return capacity_; }

    void reserve(size_type new_capacity) {
        if (new_capacity > capacity_) {
            reallocate(new_capacity);
        }
    }

    void shrink_to_fit() {
        if (size_ < capacity_) {
            reallocate(size_);
        }
    }

    // 修改操作
    void clear() noexcept {
        if (data_) {
            std::destroy(data_, data_ + size_);
            size_ = 0;
        }
    }

    iterator insert(const_iterator pos, const T& value) {
        return emplace(pos, value);
    }

    iterator insert(const_iterator pos, T&& value) {
        return emplace(pos, std::move(value));
    }

    iterator erase(const_iterator pos) {
        assert(pos >= begin() && pos < end());
        iterator non_const_pos = const_cast<iterator>(pos);
        std::move(non_const_pos + 1, end(), non_const_pos);
        --size_;
        std::destroy_at(data_ + size_);
        return non_const_pos;
    }

    iterator erase(const_iterator first, const_iterator last) {
        assert(first <= last && first >= begin() && last <= end());
        if (first == last) return const_cast<iterator>(first);

        iterator non_const_first = const_cast<iterator>(first);
        iterator non_const_last = const_cast<iterator>(last);
        iterator new_end = std::move(non_const_last, end(), non_const_first);
        std::destroy(new_end, end());
        size_ -= (last - first);
        return non_const_first;
    }

    void push_back(const T& value) {
        emplace_back(value);
    }

    void push_back(T&& value) {
        emplace_back(std::move(value));
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        if (size_ >= capacity_) {
            reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        std::allocator_traits<Allocator>::construct(
            alloc_, data_ + size_, std::forward<Args>(args)...);
        ++size_;
        return back();
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        assert(pos >= begin() && pos <= end());
        if (pos == end()) {
            emplace_back(std::forward<Args>(args)...);
            return end() - 1;
        }

        // 在 pos 处构造新元素，并移动后续元素
        if (size_ >= capacity_) {
            size_type offset = pos - begin();
            reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
            pos = begin() + offset;
        }

        iterator non_const_pos = const_cast<iterator>(pos);
        if (non_const_pos != end()) {
            // 移动最后一个元素到新位置（未初始化内存）
            std::allocator_traits<Allocator>::construct(
                alloc_, data_ + size_, std::move(data_[size_ - 1]));
            // 移动中间元素
            std::move_backward(non_const_pos, end() - 1, end());
            // 销毁原位置的元素
            std::destroy_at(non_const_pos);
        }
        // 在 pos 处构造新元素
        std::allocator_traits<Allocator>::construct(
            alloc_, non_const_pos, std::forward<Args>(args)...);
        ++size_;
        return non_const_pos;
    }

    void pop_back() {
        if (size_ > 0) {
            --size_;
            std::destroy_at(data_ + size_);
        }
    }

    void resize(size_type new_size, const T& value = T()) {
        if (new_size > size_) {
            if (new_size > capacity_) {
                reallocate(new_size);
            }
            std::uninitialized_fill(data_ + size_, data_ + new_size, value);
        } else if (new_size < size_) {
            std::destroy(data_ + new_size, data_ + size_);
        }
        size_ = new_size;
    }

    void swap(vector& other) noexcept {
        using std::swap;
        if constexpr (std::allocator_traits<Allocator>::propagate_on_container_swap::value) {
            swap(alloc_, other.alloc_);
        }
        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    }

private:
    // 分配内存（使用分配器）
    pointer allocate(size_type n) {
        if (n > max_size()) {
            throw std::length_error("vector::allocate - requested size exceeds max_size()");
        }
        return std::allocator_traits<Allocator>::allocate(alloc_, n);
    }

    // 释放内存（使用分配器）
    void deallocate(pointer p, size_type n) noexcept {
        if (p) {
            std::allocator_traits<Allocator>::deallocate(alloc_, p, n);
        }
    }

    // 重新分配内存（扩容或缩容）
    void reallocate(size_type new_capacity) {
        pointer new_data = allocate(new_capacity);
        if (data_) {
            // 移动现有元素到新内存
            std::uninitialized_move(data_, data_ + size_, new_data);
            // 销毁旧元素
            std::destroy(data_, data_ + size_);
            deallocate(data_, capacity_);
        }
        data_ = new_data;
        capacity_ = new_capacity;
    }

    // 最大可能大小
    size_type max_size() const noexcept {
        return std::allocator_traits<Allocator>::max_size(alloc_);
    }

private:
    Allocator alloc_;   // 分配器
    pointer data_;      // 动态数组指针
    size_type size_;    // 当前元素数量
    size_type capacity_;// 当前分配的内存容量
};

// 非成员函数
template <typename T, typename Alloc>
void swap(vector<T, Alloc>& lhs, vector<T, Alloc>& rhs) noexcept {
    lhs.swap(rhs);
}