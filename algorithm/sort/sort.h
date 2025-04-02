#ifndef __SORT_H__
#define __SORT_H__

#include <stddef.h>

// 回调比较函数类型定义
// 返回值: <0表示a<b, 0表示a==b, >0表示a>b
typedef int (*compare_func_t)(const void *a, const void *b, void *context);

// 排序算法类型枚举
typedef enum {
    SORT_AUTO = 0,     // 自动选择排序算法
    SORT_BUBBLE,       // 冒泡排序
    SORT_SELECTION,    // 选择排序
    SORT_INSERTION,    // 插入排序
    SORT_QUICK,        // 快速排序
    SORT_MERGE,        // 归并排序
    SORT_HEAP,         // 堆排序
    SORT_COUNT         // 排序算法数量
} sort_algorithm_t;

// 主排序函数 - 类似qsort_s，自动选择算法
// base: 要排序的数组
// num: 数组中元素个数
// size: 每个元素的大小（字节数）
// compare: 比较函数
// context: 传递给比较函数的上下文
// algorithm: 指定排序算法，SORT_AUTO为自动选择
extern int sort(void *base, size_t num, size_t size,
                compare_func_t compare, void *context,
                sort_algorithm_t algorithm);

// 优化的冒泡排序 (增加了提前退出的机制)
extern int optimized_bubble_sort(void *base, size_t num, size_t size,
                                compare_func_t compare, void *context);

// 选择排序
extern int selection_sort(void *base, size_t num, size_t size,
                         compare_func_t compare, void *context);

// 插入排序
extern int insertion_sort(void *base, size_t num, size_t size,
                         compare_func_t compare, void *context);

// 快速排序
extern int quick_sort(void *base, size_t num, size_t size,
                     compare_func_t compare, void *context);

// 归并排序
extern int merge_sort(void *base, size_t num, size_t size,
                     compare_func_t compare, void *context);

// 堆排序
extern int heap_sort(void *base, size_t num, size_t size,
                    compare_func_t compare, void *context);

// 获取排序算法名称
extern const char *get_sort_algorithm_name(sort_algorithm_t algorithm);

// 推荐排序算法 (根据数据量和特性)
extern sort_algorithm_t recommend_sort_algorithm(size_t num, size_t size);

// 设置排序算法选择策略
typedef sort_algorithm_t (*algorithm_selector_t)(size_t num, size_t size);
extern void set_algorithm_selector(algorithm_selector_t selector);

#endif /* __SORT_H__ */