#include "priority_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 整数析构函数
void int_destructor(void *element)
{
    if (element) {
        int value = (long)element;
        printf("销毁整数: %d\n", value);
        // 注意：这里不需要free，因为我们用的是指针转换而不是堆分配的内存
    }
}

// 字符串析构函数
void string_destructor(void *element)
{
    if (element) {
        const char *str = (const char*)element;
        printf("销毁字符串: %s\n", str);
        // 注意：这里不释放字符串内存，因为示例中的字符串是静态分配的
    }
}

// 堆分配数据的析构函数
void heap_data_destructor(void *element)
{
    if (element) {
        int *value = (int*)element;
        printf("销毁堆数据: %d\n", *value);
        free(element); // 释放堆内存
    }
}

// 自定义比较函数 - 按字符串长度比较
int compare_by_length(const void *a, const void *b, void *arg) 
{
    pq_type_t type = *(pq_type_t*)arg;
    const char *str_a = (const char*)a;
    const char *str_b = (const char*)b;
    
    int diff = strlen(str_a) - strlen(str_b);
    // 根据堆类型返回不同的比较结果
    return type == PQ_MIN_HEAP ? -diff : diff;
}

// 基本使用示例
void basic_usage_example()
{
    printf("\n===== 基本使用示例 =====\n");
    
    // 创建最大堆，使用默认比较函数
    pq_t *max_heap = pq_create(10, PQ_MAX_HEAP, NULL, NULL, NULL);
    if (!max_heap) {
        printf("创建最大堆失败\n");
        return;
    }
    
    // 存储一些整数值（将整数转换为指针）
    int values[] = {3, 1, 7, 5, 8, 2, 6, 4};
    int n = sizeof(values) / sizeof(values[0]);
    
    printf("输入数据: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", values[i]);
        pq_status_t status = pq_push(max_heap, (void*)(long)values[i]);
        if (status != PQ_SUCCESS) {
            printf("\n入队失败: %d\n", values[i]);
        }
    }
    printf("\n");
    
    printf("堆大小: %zu/%zu\n", pq_size(max_heap), pq_capacity(max_heap));
    
    // 查看堆顶元素
    void *top;
    if (pq_peek(max_heap, &top) == PQ_SUCCESS) {
        printf("堆顶元素: %ld\n", (long)top);
    }
    
    // 出队
    printf("出队元素: ");
    while (!pq_is_empty(max_heap)) {
        void *element;
        if (pq_peek(max_heap, &element) == PQ_SUCCESS) {
            printf("%ld ", (long)element);
            pq_pop(max_heap);
        }
    }
    printf("\n");
    
    // 销毁最大堆
    pq_destroy(max_heap);
}

// 自动析构函数示例
void destructor_example()
{
    printf("\n===== 析构函数示例 =====\n");
    
    // 创建带析构函数的最小堆
    pq_t *min_heap = pq_create(10, PQ_MIN_HEAP, NULL, NULL, int_destructor);
    if (!min_heap) {
        printf("创建最小堆失败\n");
        return;
    }
    
    // 添加一些数据
    int values[] = {30, 10, 70, 50, 80, 20, 60, 40};
    int n = sizeof(values) / sizeof(values[0]);
    
    printf("添加数据: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", values[i]);
        pq_push(min_heap, (void*)(long)values[i]);
    }
    printf("\n");
    
    // 出队部分元素（调用析构函数）
    printf("\n出队前3个元素（调用析构函数）:\n");
    for (int i = 0; i < 3; i++) {
        void *element;
        if (pq_pop(min_heap) == PQ_SUCCESS) {
            // element已被析构，这里不做额外操作
        }
    }
    
    // 清空队列（会调用析构函数）
    printf("\n清空队列（会调用析构函数）:\n");
    pq_clear(min_heap);
    
    // 销毁堆
    pq_destroy(min_heap);
}

// 堆分配数据示例
void heap_allocation_example()
{
    printf("\n===== 堆分配数据示例 =====\n");
    
    // 创建带析构函数的最大堆
    pq_t *heap = pq_create(10, PQ_MAX_HEAP, NULL, NULL, heap_data_destructor);
    if (!heap) {
        printf("创建堆失败\n");
        return;
    }
    
    // 添加堆分配的数据
    printf("添加堆分配数据:\n");
    for (int i = 1; i <= 5; i++) {
        int *value = (int*)malloc(sizeof(int));
        if (!value) {
            printf("内存分配失败\n");
            continue;
        }
        
        *value = i * 10;
        printf("  添加: %d\n", *value);
        
        pq_push(heap, value);
    }
    
    // 出队并检查内存释放
    printf("\n出队元素 (自动释放内存):\n");
    while (!pq_is_empty(heap)) {
        void *element;
        pq_pop(heap); // 会自动调用析构函数释放内存
    }
    
    // 销毁堆
    pq_destroy(heap);
}

// 字符串排序示例
void string_sorting_example()
{
    printf("\n===== 字符串排序示例 =====\n");
    
    // 创建最大堆，按字符串长度排序
    pq_t *str_heap = pq_create(10, PQ_MAX_HEAP, compare_by_length, NULL, string_destructor);
    if (!str_heap) {
        printf("创建堆失败\n");
        return;
    }
    
    // 存储一些字符串
    const char *strings[] = {"a", "abc", "abcd", "ab", "abcde", "abcdef"};
    int str_count = sizeof(strings) / sizeof(strings[0]);
    
    printf("输入字符串: ");
    for (int i = 0; i < str_count; i++) {
        printf("%s(%zu) ", strings[i], strlen(strings[i]));
        pq_push(str_heap, (void*)strings[i]);
    }
    printf("\n");
    
    // 出队并打印
    printf("按长度排序: ");
    while (!pq_is_empty(str_heap)) {
        void *element;
        if (pq_peek(str_heap, &element) == PQ_SUCCESS) {
            const char *str = (const char*)element;
            printf("%s(%zu) ", str, strlen(str));
            pq_pop(str_heap);
        }
    }
    printf("\n");
    
    // 销毁堆
    pq_destroy(str_heap);
}

// 错误处理示例
void error_handling_example()
{
    printf("\n===== 错误处理示例 =====\n");
    
    // 创建堆
    pq_t *heap = pq_create(3, PQ_MAX_HEAP, NULL, NULL, NULL);
    if (!heap) {
        printf("创建堆失败\n");
        return;
    }
    
    // 添加元素直到满
    printf("添加元素直到堆满...\n");
    for (int i = 1; i <= 5; i++) {
        pq_status_t status = pq_push(heap, (void*)(long)(i * 10));
        printf("添加 %d: %s\n", i * 10, 
              status == PQ_SUCCESS ? "成功" : 
              status == PQ_FULL ? "堆已满(自动扩容)" : "失败");
    }
    
    printf("\n堆大小: %zu/%zu\n", pq_size(heap), pq_capacity(heap));
    
    // 清空堆
    pq_clear(heap);
    printf("清空后堆大小: %zu\n", pq_size(heap));
    
    // 尝试从空堆出队
    void *element;
    pq_status_t status = pq_pop(heap);
    printf("从空堆出队: %s\n", 
          status == PQ_SUCCESS ? "成功" : 
          status == PQ_EMPTY ? "堆为空" : "错误");
    
    // 销毁堆
    pq_destroy(heap);
}

int main() 
{
    basic_usage_example();
    destructor_example();
    heap_allocation_example();
    string_sorting_example();
    error_handling_example();
    
    return 0;
}