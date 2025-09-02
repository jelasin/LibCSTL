#include "priority_queue.h"
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#define PQ_DEFAULT_CAPACITY 16
#define PQ_RESIZE_FACTOR    2

typedef void* (*malloc_func)(size_t size);
typedef void (*free_func)(void* ptr);

static malloc_func malloc_hook = malloc;
static free_func free_hook = free;

// 设置内存分配函数
void pq_set_memory_alloc(void* (*alloc)(size_t size))
{
    malloc_hook = alloc;
}

// 设置内存释放函数
void pq_set_memory_free(void (*release)(void* ptr))
{
    free_hook = release;
}

// 默认比较函数 (用于直接比较整数等值类型)
static int default_compare(const void *a, const void *b, void *arg) 
{
    pq_type_t type = *(pq_type_t*)arg;
    long long diff = (long long)a - (long long)b;
    return type == PQ_MIN_HEAP ? -diff : diff;
}

// 父节点索引
static inline size_t parent_index(size_t i) 
{ 
    return (i - 1) / 2; 
}

// 左子节点索引
static inline size_t left_child_index(size_t i) 
{ 
    return 2 * i + 1; 
}

// 右子节点索引
static inline size_t right_child_index(size_t i) 
{ 
    return 2 * i + 2; 
}

// 交换两个元素
static inline void swap(void **array, size_t i, size_t j) 
{
    void *temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

// 创建优先队列
pq_t* pq_create(size_t initial_capacity, pq_type_t type, 
               int (*compare)(const void *a, const void *b, void *arg), 
               void *compare_arg,
               void (*element_destructor)(void *element)) 
{
    // 参数检查
    if (initial_capacity == 0) {
#ifdef DEBUG
        perror("[pq_create] initial_capacity cannot be 0");
#endif
        return NULL;
    }
    
    pq_t *pq = (pq_t*)malloc_hook(sizeof(pq_t));
    if (!pq) {
#ifdef DEBUG
        perror("[pq_create] malloc_hook failed");
#endif
        return NULL;
    }

    // 初始化容量 (至少为默认容量)
    if (initial_capacity < PQ_DEFAULT_CAPACITY) {
        initial_capacity = PQ_DEFAULT_CAPACITY;
    }

    // 分配数组空间
    pq->array = (void**)malloc_hook(initial_capacity * sizeof(void*));
    if (!pq->array) {
        free_hook(pq);
        return NULL;
    }

    // 初始化结构体字段
    pq->size = 0;
    pq->capacity = initial_capacity;
    pq->type = type;
    pq->element_destructor = element_destructor;
    
    // 如果没有提供比较函数，使用默认的
    pq->compare = compare ? compare : default_compare;
    pq->compare_arg = compare_arg ? compare_arg : &pq->type;

    return pq;
}

// 销毁优先队列
void pq_destroy(pq_t *pq) 
{
    if (!pq) {
#ifdef DEBUG
        perror("[pq_destroy] pq is NULL");
#endif
        return;
    }
    
    // 清空队列，会调用析构函数
    pq_clear(pq);
    
    // 释放资源
    free_hook(pq->array);
    pq->array = NULL;
    free_hook(pq);
}

// 清空优先队列
void pq_clear(pq_t *pq) 
{
    if (!pq) {
#ifdef DEBUG
        perror("[pq_clear] pq is NULL");
#endif
        return;
    }
    
    // 如果有析构函数，则对每个元素调用它
    if (pq->element_destructor) {
        for (size_t i = 0; i < pq->size; i++) {
            if (pq->array[i]) {
                pq->element_destructor(pq->array[i]);
            }
        }
    }
    
    // 重置大小
    pq->size = 0;
}

// 调整队列的容量
pq_status_t pq_reserve(pq_t *pq, size_t new_capacity) 
{
    if (!pq) {
#ifdef DEBUG
        perror("[pq_reserve] pq is NULL");
#endif
        return PQ_ERROR;
    }
    
    // 新容量必须至少能容纳当前所有元素
    if (new_capacity < pq->size) {
#ifdef DEBUG
        perror("[pq_reserve] new_capacity cannot be less than current size");
#endif
        return PQ_ERROR;
    }
    
    // 如果新容量与当前容量相同，不需要调整
    if (new_capacity == pq->capacity) {
        return PQ_SUCCESS;
    }
    
    // 重新分配内存
    void **new_array = (void**)realloc(pq->array, new_capacity * sizeof(void*));
    if (!new_array) {
#ifdef DEBUG
        perror("[pq_reserve] realloc failed");
#endif
        return PQ_ERROR;
    }
    
    pq->array = new_array;
    pq->capacity = new_capacity;
    return PQ_SUCCESS;
}

// 上浮操作
void pq_sift_up(pq_t *pq, size_t index) 
{
    if (!pq || index >= pq->size) {
        return;
    }
    
    while (index > 0) {
        size_t p = parent_index(index);
        
        // 如果当前节点比父节点优先级低，则停止上浮
        if (pq->compare(pq->array[index], pq->array[p], pq->compare_arg) <= 0) {
            break;
        }
        
        // 交换并继续上浮
        swap(pq->array, index, p);
        index = p;
    }
}

// 下沉操作
void pq_sift_down(pq_t *pq, size_t index) 
{
    if (!pq || index >= pq->size) {
        return;
    }
    
    size_t size = pq->size;
    
    while (1) {
        size_t left = left_child_index(index);
        size_t right = right_child_index(index);
        size_t largest = index;
        
        // 找出当前节点、左子节点和右子节点中优先级最高的
        if (left < size && 
            pq->compare(pq->array[left], pq->array[largest], pq->compare_arg) > 0) {
            largest = left;
        }
        
        if (right < size && 
            pq->compare(pq->array[right], pq->array[largest], pq->compare_arg) > 0) {
            largest = right;
        }
        
        // 如果当前节点已经是最高优先级，则停止下沉
        if (largest == index) {
            break;
        }
        
        // 交换并继续下沉
        swap(pq->array, index, largest);
        index = largest;
    }
}

// 入队 (添加元素)
pq_status_t pq_push(pq_t *pq, void *data) 
{
    if (!pq || !data) {
#ifdef DEBUG
        perror("[pq_push] pq or data is NULL");
#endif
        return PQ_ERROR;
    }
    
    // 检查是否需要扩容
    if (pq_is_full(pq)) {
        size_t new_capacity = pq->capacity * PQ_RESIZE_FACTOR;
        pq_status_t status = pq_reserve(pq, new_capacity);
        if (status != PQ_SUCCESS) {
            return status;
        }
    }
    
    // 将新元素添加到末尾
    pq->array[pq->size++] = data;
    
    // 调整堆，使其满足堆性质
    pq_sift_up(pq, pq->size - 1);
    
    return PQ_SUCCESS;
}

// 出队 (删除堆顶元素并返回)
pq_status_t pq_pop(pq_t *pq) 
{
    if (!pq) {
#ifdef DEBUG
        perror("[pq_pop] pq is NULL");
#endif
        return PQ_ERROR;
    }
    
    // 检查队列是否为空
    if (pq_is_empty(pq)) {
        return PQ_EMPTY;
    }
    
    // 记录堆顶元素（暂不删除）
    void *top_element = pq->array[0];
    
    // 用最后一个元素替换堆顶
    pq->array[0] = pq->array[--pq->size];
    
    // 如果堆不为空，则下沉新的堆顶元素
    if (pq->size > 0) {
        pq_sift_down(pq, 0);
    }
        
    return PQ_SUCCESS;
}

// 查看堆顶元素但不移除
pq_status_t pq_peek(const pq_t *pq, void **element) 
{
    if (!pq || !element) {
#ifdef DEBUG
        perror("[pq_peek] pq or element is NULL");
#endif
        return PQ_ERROR;
    }
    
    // 检查队列是否为空
    if (pq_is_empty(pq)) {
        *element = NULL;
        return PQ_EMPTY;
    }
    
    // 返回堆顶元素
    *element = pq->array[0];
    return PQ_SUCCESS;
}

// 获取队列大小
size_t pq_size(const pq_t *pq) 
{
    return pq ? pq->size : 0;
}

// 获取队列容量
size_t pq_capacity(const pq_t *pq)
{
    return pq ? pq->capacity : 0;
}

// 判断队列是否为空
int pq_is_empty(const pq_t *pq) 
{
    return (!pq || pq->size == 0);
}

// 判断队列是否已满
int pq_is_full(const pq_t *pq)
{
    return (pq && pq->size >= pq->capacity);
}

