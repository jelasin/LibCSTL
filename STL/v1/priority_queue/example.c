#include "priority_queue.h"
#include <stdio.h>
#include <string.h>

// 自定义比较函数 - 按字符串长度比较 (演示自定义compare函数)
int compare_by_length(const void *a, const void *b, void *arg) 
{
    pq_type_t type = *(pq_type_t*)arg;
    const char *str_a = (const char*)a;
    const char *str_b = (const char*)b;
    
    int diff = strlen(str_a) - strlen(str_b);
    // 根据堆类型返回不同的比较结果
    return type == PQ_MIN_HEAP ? -diff : diff;
}

int main() 
{
    printf("===== 测试最大堆 =====\n");
    // 创建最大堆，使用默认比较函数（直接比较指针值）
    pq_t *max_heap = pq_create(10, PQ_MAX_HEAP, NULL, NULL);
    
    // 存储一些整数值（将整数转换为指针）
    int values[] = {3, 1, 7, 5, 8, 2, 6, 4};
    int n = sizeof(values) / sizeof(values[0]);
    
    printf("输入数据: ");
    for (int i = 0; i < n; i++) 
    {
        printf("%d ", values[i]);
        pq_push(max_heap, (void*)(long)values[i]);
    }
    printf("\n");
    
    // 出队并打印
    printf("最大堆输出: ");
    while (!pq_empty(max_heap)) 
    {
        long val = (long)pq_pop(max_heap);
        printf("%ld ", val);
    }
    printf("\n\n");
    
    // 销毁最大堆
    pq_destroy(max_heap);
    
    printf("===== 测试最小堆 =====\n");
    // 创建最小堆
    pq_t *min_heap = pq_create(10, PQ_MIN_HEAP, NULL, NULL);
    
    printf("输入数据: ");
    for (int i = 0; i < n; i++) 
    {
        printf("%d ", values[i]);
        pq_push(min_heap, (void*)(long)values[i]);
    }
    printf("\n");
    
    // 出队并打印
    printf("最小堆输出: ");
    while (!pq_empty(min_heap)) 
    {
        long val = (long)pq_pop(min_heap);
        printf("%ld ", val);
    }
    printf("\n\n");
    
    // 销毁最小堆
    pq_destroy(min_heap);
    
    printf("===== 测试字符串长度排序 =====\n");
    // 创建自定义比较函数的最大堆 (按字符串长度降序)
    pq_t *str_heap = pq_create(10, PQ_MAX_HEAP, compare_by_length, NULL);
    
    // 存储一些字符串
    const char *strings[] = {"a", "abc", "abcd", "ab", "abcde", "abcdef"};
    int str_count = sizeof(strings) / sizeof(strings[0]);
    
    printf("输入字符串: ");
    for (int i = 0; i < str_count; i++) 
    {
        printf("%s(%zu) ", strings[i], strlen(strings[i]));
        pq_push(str_heap, (void*)strings[i]);
    }
    printf("\n");
    
    // 出队并打印
    printf("按长度排序: ");
    while (!pq_empty(str_heap)) 
    {
        const char *str = (const char*)pq_pop(str_heap);
        printf("%s(%zu) ", str, strlen(str));
    }
    printf("\n");
    
    // 销毁堆
    pq_destroy(str_heap);
    
    return 0;
}

// ===== 测试最大堆 =====
// 输入数据: 3 1 7 5 8 2 6 4 
// 最大堆输出: 8 7 6 5 4 3 2 1 
// 
// ===== 测试最小堆 =====
// 输入数据: 3 1 7 5 8 2 6 4 
// 最小堆输出: 1 2 3 4 5 6 7 8 
// 
// ===== 测试字符串长度排序 =====
// 输入字符串: a(1) abc(3) abcd(4) ab(2) abcde(5) abcdef(6) 
// 按长度排序: abcdef(6) abcde(5) abcd(4) abc(3) ab(2) a(1) 