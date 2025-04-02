#include "sort.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// 整数比较函数
int compare_int(const void *a, const void *b, void *context) {
    // 升序排序
    return *(const int*)a - *(const int*)b;
}

// 字符串比较函数
int compare_string(const void *a, const void *b, void *context) {
    return strcmp(*(const char**)a, *(const char**)b);
}

// 根据数据特性选择排序算法的自定义选择器
sort_algorithm_t custom_algorithm_selector(size_t num, size_t size) {
    // 小数据量使用插入排序
    if (num <= 20) {
        return SORT_INSERTION;
    }
    // 字符串使用归并排序（稳定性好）
    else if (size == sizeof(char*)) {
        return SORT_MERGE;
    }
    // 其他情况使用快速排序
    else {
        return SORT_QUICK;
    }
}

// 测试各种排序算法的性能
void benchmark_sort_algorithms(int *data, size_t size) {
    // 为每个算法准备一份数据副本
    int *arrays[SORT_COUNT];
    for (int i = 0; i < SORT_COUNT; i++) {
        if (i == SORT_AUTO) continue; // 跳过AUTO
        arrays[i] = malloc(size * sizeof(int));
        memcpy(arrays[i], data, size * sizeof(int));
    }
    
    // 测试每种排序算法
    for (int algorithm = SORT_BUBBLE; algorithm < SORT_COUNT; algorithm++) {
        clock_t start = clock();
        
        sort(arrays[algorithm], size, sizeof(int), compare_int, NULL, algorithm);
        
        clock_t end = clock();
        double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
        
        printf("%-15s: %f 秒\n", get_sort_algorithm_name(algorithm), time_spent);
    }
    
    // 释放内存
    for (int i = 0; i < SORT_COUNT; i++) {
        if (i == SORT_AUTO) continue;
        free(arrays[i]);
    }
}

int main() {
    // 设置自定义算法选择器, 如果不设置则使用默认选择器
    set_algorithm_selector(custom_algorithm_selector);
    
    printf("=== 整数排序测试 ===\n");
    int numbers[] = {9, 2, 7, 1, 5, 3, 8, 6, 4};
    int count = sizeof(numbers) / sizeof(numbers[0]);
    
    printf("排序前: ");
    for (int i = 0; i < count; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");
    
    // 使用自动选择的算法排序
    sort_algorithm_t chosen = recommend_sort_algorithm(count, sizeof(int));
    printf("自动选择的算法: %s\n", get_sort_algorithm_name(chosen));
    
    sort(numbers, count, sizeof(int), compare_int, NULL, SORT_AUTO);
    
    printf("排序后: ");
    for (int i = 0; i < count; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n\n");
    
    // 字符串排序测试
    printf("=== 字符串排序测试 ===\n");
    const char *strings[] = {"banana", "apple", "orange", "grape", "pear", "kiwi"};
    count = sizeof(strings) / sizeof(strings[0]);
    
    printf("排序前: ");
    for (int i = 0; i < count; i++) {
        printf("%s ", strings[i]);
    }
    printf("\n");
    
    // 使用自动选择的算法排序
    chosen = recommend_sort_algorithm(count, sizeof(char*));
    printf("自动选择的算法: %s\n", get_sort_algorithm_name(chosen));
    
    sort(strings, count, sizeof(char*), compare_string, NULL, SORT_AUTO);
    
    printf("排序后: ");
    for (int i = 0; i < count; i++) {
        printf("%s ", strings[i]);
    }
    printf("\n\n");
    
    // 性能测试
    printf("=== 性能测试 (10000个随机整数) ===\n");
    const int test_size = 10000;
    int *large_array = malloc(test_size * sizeof(int));
    
    // 生成随机数据
    srand(time(NULL));
    for (int i = 0; i < test_size; i++) {
        large_array[i] = rand() % 100000;
    }
    
    benchmark_sort_algorithms(large_array, test_size);
    
    free(large_array);
    
    return 0;
}

// === 整数排序测试 ===
// 排序前: 9 2 7 1 5 3 8 6 4 
// 自动选择的算法: Insertion Sort
// 排序后: 1 2 3 4 5 6 7 8 9 

// === 字符串排序测试 ===
// 排序前: banana apple orange grape pear kiwi 
// 自动选择的算法: Insertion Sort
// 排序后: apple banana grape kiwi orange pear 

// === 性能测试 (10000个随机整数) ===
// Bubble Sort    : 0.556736 秒
// Selection Sort : 0.294588 秒
// Insertion Sort : 0.174973 秒
// Quick Sort     : 0.002076 秒
// Merge Sort     : 0.002193 秒
// Heap Sort      : 0.003803 秒