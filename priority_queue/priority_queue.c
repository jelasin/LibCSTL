#include "priority_queue.h"
#include <stdlib.h>
#include <string.h>

#define PQ_DEFAULT_CAPACITY 16
#define PQ_RESIZE_FACTOR    2

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
                void *compare_arg) 
{
    pq_t *pq = (pq_t*)malloc(sizeof(pq_t));
    if (!pq) 
    {
        return NULL;
    }

    // 初始化容量 (至少为默认容量)
    if (initial_capacity < PQ_DEFAULT_CAPACITY)
    {
        initial_capacity = PQ_DEFAULT_CAPACITY;
    }

    // 分配数组空间
    pq->array = (void**)malloc(initial_capacity * sizeof(void*));
    if (!pq->array) 
    {
        free(pq);
        return NULL;
    }

    pq->size = 0;
    pq->capacity = initial_capacity;
    pq->type = type;
    
    // 如果没有提供比较函数，使用默认的
    pq->compare = compare ? compare : default_compare;
    pq->compare_arg = compare_arg ? compare_arg : &pq->type;

    return pq;
}

// 销毁优先队列
void pq_destroy(pq_t *pq) 
{
    if (pq) {
        free(pq->array);
        free(pq);
    }
}

// 清空优先队列
void pq_clear(pq_t *pq) 
{
    if (pq) {
        pq->size = 0;
    }
}

// 调整队列的容量
int pq_reserve(pq_t *pq, size_t new_capacity) 
{
    if (!pq || new_capacity < pq->size) return -1;
    
    void **new_array = (void**)realloc(pq->array, new_capacity * sizeof(void*));
    if (!new_array) return -1;
    
    pq->array = new_array;
    pq->capacity = new_capacity;
    return 0;
}

// 上浮操作
void pq_sift_up(pq_t *pq, size_t index) 
{
    while (index > 0) {
        size_t p = parent_index(index);
        
        // 如果当前节点比父节点优先级低，则停止上浮
        if (pq->compare(pq->array[index], pq->array[p], pq->compare_arg) <= 0)
            break;
        
        // 交换并继续上浮
        swap(pq->array, index, p);
        index = p;
    }
}

// 下沉操作
void pq_sift_down(pq_t *pq, size_t index) 
{
    size_t size = pq->size;
    
    while (1) {
        size_t left = left_child_index(index);
        size_t right = right_child_index(index);
        size_t largest = index;
        
        // 找出当前节点、左子节点和右子节点中优先级最高的
        if (left < size && 
            pq->compare(pq->array[left], pq->array[largest], pq->compare_arg) > 0)
            largest = left;
        
        if (right < size && 
            pq->compare(pq->array[right], pq->array[largest], pq->compare_arg) > 0)
            largest = right;
        
        // 如果当前节点已经是最高优先级，则停止下沉
        if (largest == index)
            break;
        
        // 交换并继续下沉
        swap(pq->array, index, largest);
        index = largest;
    }
}

// 入队 (添加元素)
int pq_push(pq_t *pq, void *data) 
{
    if (!pq) return -1;
    
    // 如果队列已满，扩容
    if (pq->size >= pq->capacity) {
        size_t new_capacity = pq->capacity * PQ_RESIZE_FACTOR;
        if (pq_reserve(pq, new_capacity) != 0)
            return -1;
    }
    
    // 将新元素添加到末尾
    pq->array[pq->size++] = data;
    
    // 调整堆，使其满足堆性质
    pq_sift_up(pq, pq->size - 1);
    
    return 0;
}

// 出队 (删除堆顶元素并返回)
void* pq_pop(pq_t *pq) 
{
    if (!pq || pq->size == 0) return NULL;
    
    // 保存堆顶元素
    void *top = pq->array[0];
    
    // 用最后一个元素替换堆顶
    pq->array[0] = pq->array[--pq->size];
    
    // 如果堆不为空，则下沉
    if (pq->size > 0)
        pq_sift_down(pq, 0);
    
    return top;
}

// 查看堆顶元素
void* pq_top(const pq_t *pq) 
{
    if (!pq || pq->size == 0) return NULL;
    return pq->array[0];
}

// 获取队列大小
size_t pq_size(const pq_t *pq) 
{
    return pq ? pq->size : 0;
}

// 判断队列是否为空
int pq_empty(const pq_t *pq) 
{
    return pq ? pq->size == 0 : 1;
}

