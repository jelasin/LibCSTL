#include "sort.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// 默认的算法选择器
static algorithm_selector_t g_algorithm_selector = NULL;

// 排序算法名称
static const char *sort_algorithm_names[] = {
    "Auto",
    "Bubble Sort",
    "Selection Sort",
    "Insertion Sort",
    "Quick Sort",
    "Merge Sort",
    "Heap Sort"
};

// 内部工具函数: 交换元素
static void swap_elements(void *a, void *b, size_t size) 
{
    // 使用临时缓冲区交换元素
    char temp[size];
    memcpy(temp, a, size);
    memcpy(a, b, size);
    memcpy(b, temp, size);
}

// 内部工具函数: 获取数组中的元素指针
static inline void *get_element(void *base, size_t index, size_t size) 
{
    return (char *)base + index * size;
}

// 默认的算法选择逻辑
static sort_algorithm_t default_algorithm_selector(size_t num, size_t size) 
{
    // 根据数据量选择合适的排序算法
    if (num <= 10) {
        return SORT_INSERTION;  // 数据量很小时，插入排序性能好
    } else if (num <= 100) {
        return SORT_QUICK;      // 中等数据量，快速排序
    } else if (num <= 1000) {
        return SORT_MERGE;      // 较大数据量，稳定的归并排序
    } else {
        return SORT_QUICK;      // 大数据量，快速排序通常最快
    }
}

// 推荐排序算法
sort_algorithm_t recommend_sort_algorithm(size_t num, size_t size) 
{
    if (g_algorithm_selector) 
    {
        return g_algorithm_selector(num, size);
    }
    return default_algorithm_selector(num, size);
}

// 设置算法选择器
void set_algorithm_selector(algorithm_selector_t selector) 
{
    g_algorithm_selector = selector;
}

// 获取排序算法名称
const char *get_sort_algorithm_name(sort_algorithm_t algorithm) 
{
    if (algorithm >= 0 && algorithm < SORT_COUNT) 
    {
        return sort_algorithm_names[algorithm];
    }
    return "Unknown";
}

//-----------------------------
// 各种排序算法实现
//-----------------------------

// 1. 优化的冒泡排序
int optimized_bubble_sort(void *base, size_t num, size_t size, 
                         compare_func_t compare, void *context) 
{
    if (!base || !compare || num < 2) return 0;
    
    for (size_t i = 0; i < num - 1; i++) 
    {
        bool swapped = false;
        
        for (size_t j = 0; j < num - i - 1; j++) 
        {
            void *elem1 = get_element(base, j, size);
            void *elem2 = get_element(base, j + 1, size);
            
            if (compare(elem1, elem2, context) > 0) 
            {
                swap_elements(elem1, elem2, size);
                swapped = true;
            }
        }
        
        // 如果本轮没有交换元素，说明数组已经有序
        if (!swapped)
        {
            break;
        }
    }
    
    return 0;
}

// 2. 选择排序
int selection_sort(void *base, size_t num, size_t size, 
                  compare_func_t compare, void *context) 
{
    if (!base || !compare || num < 2) 
    {
        return 0;
    }
    
    for (size_t i = 0; i < num - 1; i++) 
    {
        size_t min_idx = i;
        
        for (size_t j = i + 1; j < num; j++) 
        {
            void *curr_min = get_element(base, min_idx, size);
            void *curr_elem = get_element(base, j, size);
            
            if (compare(curr_elem, curr_min, context) < 0) 
            {
                min_idx = j;
            }
        }
        
        if (min_idx != i) 
        {
            void *elem1 = get_element(base, i, size);
            void *elem2 = get_element(base, min_idx, size);
            swap_elements(elem1, elem2, size);
        }
    }
    
    return 0;
}

// 3. 插入排序
int insertion_sort(void *base, size_t num, size_t size, 
                  compare_func_t compare, void *context) 
{
    if (!base || !compare || num < 2) 
    {
        return 0;
    }
    
    char key[size];  // 临时存储当前要插入的元素
    
    for (size_t i = 1; i < num; i++) 
    {
        void *curr = get_element(base, i, size);
        memcpy(key, curr, size);  // 保存当前元素
        
        int j = i - 1;
        void *prev;
        
        // 将所有大于key的元素向后移动
        while (j >= 0 && (prev = get_element(base, j, size), 
                          compare(prev, key, context) > 0)) 
        {
            void *next = get_element(base, j + 1, size);
            memcpy(next, prev, size);
            j--;
        }
        
        // 将key放到正确的位置
        void *pos = get_element(base, j + 1, size);
        memcpy(pos, key, size);
    }
    
    return 0;
}

// 快速排序 (内部递归实现)
static void quick_sort_recursive(void *base, int low, int high, size_t size,
                               compare_func_t compare, void *context) 
{
    if (low < high) 
    {
        // 如果区间很小，使用插入排序
        if (high - low < 10) 
        {
            // 对子数组使用插入排序
            for (int i = low + 1; i <= high; i++) 
            {
                void *curr = get_element(base, i, size);
                char key[size];
                memcpy(key, curr, size);
                
                int j = i - 1;
                while (j >= low && compare(get_element(base, j, size), key, context) > 0) 
                {
                    memcpy(get_element(base, j + 1, size), get_element(base, j, size), size);
                    j--;
                }
                memcpy(get_element(base, j + 1, size), key, size);
            }
            return;
        }
        
        // 分区过程
        void *pivot = get_element(base, high, size);
        char pivot_value[size];
        memcpy(pivot_value, pivot, size);
        
        int i = low - 1;
        
        for (int j = low; j < high; j++) 
        {
            void *curr = get_element(base, j, size);
            if (compare(curr, pivot_value, context) <= 0) 
            {
                i++;
                void *elem_i = get_element(base, i, size);
                swap_elements(elem_i, curr, size);
            }
        }
        
        void *elem_i1 = get_element(base, i + 1, size);
        swap_elements(elem_i1, pivot, size);
        
        int partition = i + 1;
        
        // 递归对左右子数组进行排序
        quick_sort_recursive(base, low, partition - 1, size, compare, context);
        quick_sort_recursive(base, partition + 1, high, size, compare, context);
    }
}

// 4. 快速排序 (公共接口)
int quick_sort(void *base, size_t num, size_t size,
              compare_func_t compare, void *context) 
{
    if (!base || !compare || num < 2) 
    {
        return 0;
    }
    
    quick_sort_recursive(base, 0, num - 1, size, compare, context);
    return 0;
}

// 归并排序 (内部合并函数)
static void merge(void *base, int left, int mid, int right, size_t size,
                 compare_func_t compare, void *context) 
{
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    // 创建临时数组
    void *L = malloc(n1 * size);
    void *R = malloc(n2 * size);
    
    // 复制数据到临时数组
    for (int i = 0; i < n1; i++) 
    {
        memcpy((char*)L + i * size, get_element(base, left + i, size), size);
    }
    for (int j = 0; j < n2; j++) 
    {
        memcpy((char*)R + j * size, get_element(base, mid + 1 + j, size), size);
    }
    
    // 合并临时数组
    int i = 0, j = 0, k = left;
    
    while (i < n1 && j < n2) 
    {
        void *L_elem = (char*)L + i * size;
        void *R_elem = (char*)R + j * size;
        
        if (compare(L_elem, R_elem, context) <= 0) 
        {
            memcpy(get_element(base, k, size), L_elem, size);
            i++;
        } 
        else 
        {
            memcpy(get_element(base, k, size), R_elem, size);
            j++;
        }
        k++;
    }
    
    // 复制剩余元素
    while (i < n1) 
    {
        memcpy(get_element(base, k, size), (char*)L + i * size, size);
        i++;
        k++;
    }
    while (j < n2) 
    {
        memcpy(get_element(base, k, size), (char*)R + j * size, size);
        j++;
        k++;
    }
    
    // 释放临时数组
    free(L);
    free(R);
}

// 归并排序 (内部递归实现)
static void merge_sort_recursive(void *base, int left, int right, size_t size,
                                compare_func_t compare, void *context) 
{
    if (left < right) 
    {
        if (right - left < 10) 
        {
            // 对小数组使用插入排序
            for (int i = left + 1; i <= right; i++) 
            {
                void *curr = get_element(base, i, size);
                char key[size];
                memcpy(key, curr, size);
                
                int j = i - 1;
                while (j >= left && compare(get_element(base, j, size), key, context) > 0) 
                {
                    memcpy(get_element(base, j + 1, size), get_element(base, j, size), size);
                    j--;
                }
                memcpy(get_element(base, j + 1, size), key, size);
            }
            return;
        }
        
        int mid = left + (right - left) / 2;
        
        merge_sort_recursive(base, left, mid, size, compare, context);
        merge_sort_recursive(base, mid + 1, right, size, compare, context);
        
        merge(base, left, mid, right, size, compare, context);
    }
}

// 5. 归并排序 (公共接口)
int merge_sort(void *base, size_t num, size_t size,
              compare_func_t compare, void *context) 
{
    if (!base || !compare || num < 2) 
    {
        return 0;
    }
    
    merge_sort_recursive(base, 0, num - 1, size, compare, context);
    return 0;
}

// 堆排序 (内部维护堆的函数)
static void heapify(void *base, size_t num, size_t size, size_t i,
                   compare_func_t compare, void *context) 
{
    size_t largest = i;
    size_t left = 2 * i + 1;
    size_t right = 2 * i + 2;
    
    void *largest_elem = get_element(base, largest, size);
    
    // 如果左子节点大于根节点
    if (left < num) 
    {
        void *left_elem = get_element(base, left, size);
        if (compare(left_elem, largest_elem, context) > 0) 
        {
            largest = left;
            largest_elem = left_elem;
        }
    }
    
    // 如果右子节点大于当前最大值
    if (right < num) 
    {
        void *right_elem = get_element(base, right, size);
        if (compare(right_elem, largest_elem, context) > 0) 
        {
            largest = right;
        }
    }
    
    // 如果最大值不是根节点
    if (largest != i) 
    {
        swap_elements(get_element(base, i, size), 
                      get_element(base, largest, size), size);
        
        // 递归维护子堆
        heapify(base, num, size, largest, compare, context);
    }
}

// 6. 堆排序 (公共接口)
int heap_sort(void *base, size_t num, size_t size,
             compare_func_t compare, void *context) 
{
    if (!base || !compare || num < 2) 
    {
        return 0;
    }
    
    // 构建最大堆
    for (int i = num / 2 - 1; i >= 0; i--) 
    {
        heapify(base, num, size, i, compare, context);
    }
    
    // 一个个从堆中提取元素
    for (size_t i = num - 1; i > 0; i--) 
    {
        // 将当前根节点移到末尾
        swap_elements(get_element(base, 0, size), 
                      get_element(base, i, size), size);
        
        // 对减小的堆调用heapify
        heapify(base, i, size, 0, compare, context);
    }
    
    return 0;
}

// 主排序函数 - 根据算法类型选择合适的排序方法
int sort(void *base, size_t num, size_t size,
        compare_func_t compare, void *context,
        sort_algorithm_t algorithm) 
{
    if (!base || !compare || num < 2) 
    {
        return 0;
    }
    
    // 如果是自动选择，调用推荐函数
    if (algorithm == SORT_AUTO) 
    {
        algorithm = recommend_sort_algorithm(num, size);
    }
    
    // 根据算法类型调用相应的排序函数
    switch (algorithm) 
    {
        case SORT_BUBBLE:
            return optimized_bubble_sort(base, num, size, compare, context);
        case SORT_SELECTION:
            return selection_sort(base, num, size, compare, context);
        case SORT_INSERTION:
            return insertion_sort(base, num, size, compare, context);
        case SORT_QUICK:
            return quick_sort(base, num, size, compare, context);
        case SORT_MERGE:
            return merge_sort(base, num, size, compare, context);
        case SORT_HEAP:
            return heap_sort(base, num, size, compare, context);
        default:
            return quick_sort(base, num, size, compare, context);
    }
}