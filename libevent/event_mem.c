#include <event2/event.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* 内存对齐处理 */
union alignment {
    size_t sz;   // size_t类型，通常8字节
    void *ptr;   // 指针类型，64位系统下8字节
    double dbl;  // double类型，8字节
};
#define ALIGNMENT sizeof(union alignment)  // 计算对齐大小为8字节
#define OUTPTR(ptr) (((char*)ptr) + ALIGNMENT)  // 将指针向后偏移8字节
#define INPTR(ptr) (((char*)ptr) - ALIGNMENT)   // 将指针向前偏移8字节

/* 全局内存统计 */
static size_t total_allocated = 0;

/* 替换的内存分配函数 */
static void *replacement_malloc(size_t sz)
{
    printf("replacement malloc function sz = %lu... \n", sz);
    void *chunk = malloc(sz + ALIGNMENT);  // 多分配8字节用于存储大小
    if (!chunk) return chunk;              // 分配失败返回NULL
    total_allocated += sz;                 // 增加总分配量
    *(size_t*)chunk = sz;                  // 在前8字节存储分配大小
    printf("replacement malloc function total allocaed = %lu... \n", total_allocated);
    return OUTPTR(chunk);                  // 返回偏移后的指针(跳过存储的大小)
}

static void *replacement_realloc(void *ptr, size_t sz)
{
    printf("replacement realloc function sz = %lu... \n", sz);
    size_t old_size = 0;
    if (ptr) {                       // 如果原指针不为NULL
        ptr = INPTR(ptr);            // 获取原始指针位置
        old_size = *(size_t*)ptr;    // 读取原分配大小
    }
    ptr = realloc(ptr, sz + ALIGNMENT);  // 重新分配内存
    if (!ptr) return NULL;               // 分配失败返回NULL
    *(size_t*)ptr = sz;                  // 存储新大小
    total_allocated = total_allocated - old_size + sz;  // 更新总分配量
    printf("replacement realloc function total allocaed = %lu... \n", total_allocated);
    return OUTPTR(ptr);                  // 返回偏移后的指针
}

static void replacement_free(void *ptr)
{
    printf("replacement free function... \n");
    ptr = INPTR(ptr);                  // 获取原始指针位置
    total_allocated -= *(size_t*)ptr;  // 减少总分配量
    free(ptr);                         // 释放内存
}

/* 启动内存计数 */
void start_counting_bytes(void)
{
    event_set_mem_functions(replacement_malloc,
                          replacement_realloc,
                          replacement_free);
}

/* 获取当前总分配内存量 */
size_t get_total_allocated(void)
{
    return total_allocated;
}

/* 测试用例 */
void test_memory_tracking(void)
{
    printf("Initial allocated: %zu bytes\n", get_total_allocated());
    
    // 测试malloc
    void *p1 = malloc(100);
    printf("After malloc(100): %zu bytes\n", get_total_allocated());
    
    // 测试realloc
    p1 = realloc(p1, 200);
    printf("After realloc(200): %zu bytes\n", get_total_allocated());
    
    // 测试free
    free(p1);
    printf("After free: %zu bytes\n", get_total_allocated());
    
    // 测试libevent内存分配
    struct event_base *base = event_base_new();
    printf("After event_base_new(): %zu bytes\n", get_total_allocated());
    event_base_free(base);
    printf("After event_base_free(): %zu bytes\n", get_total_allocated());
}

int main(void)
{
    char * ptr = (char *)malloc(sizeof(char));
    double db = 0;
    printf("ALIGNMENT = %lu\n", ALIGNMENT); // 8
    printf("ALIGNMENT = %lu\n", sizeof(ptr)); // 8
    printf("ALIGNMENT = %lu\n", sizeof(db));  // 8
    printf("ptr address = %p\n", ptr);
    free(ptr);
    ptr = NULL;

    // 必须在libevent任何调用之前设置内存函数
    start_counting_bytes();
    
    printf("Memory tracking started\n");
    
    // 运行测试
    test_memory_tracking();
    
    // 验证内存泄漏（最终应为0）
    assert(get_total_allocated() == 0);
    printf("All memory properly freed\n");
    
    return 0;
}