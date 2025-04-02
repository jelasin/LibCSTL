#ifndef __PRIORITY_QUEUE_H__
#define __PRIORITY_QUEUE_H__

#include <stdlib.h>

// 优先队列类型
typedef enum {
    PQ_MIN_HEAP = 0,  // 最小堆 (堆顶是最小值)
    PQ_MAX_HEAP = 1   // 最大堆 (堆顶是最大值)
} pq_type_t;

// 优先队列结构体定义
typedef struct priority_queue {
    void **array;         // 存储元素的数组
    size_t size;          // 当前元素个数
    size_t capacity;      // 数组容量
    pq_type_t type;       // 堆类型: 最小堆或最大堆
    
    // 比较函数，返回值:
    // > 0: a优先级高于b
    // = 0: a优先级等于b
    // < 0: a优先级低于b
    int (*compare)(const void *a, const void *b, void *arg);
    void *compare_arg;    // 比较函数的额外参数
} pq_t;

// 创建优先队列
extern pq_t* pq_create(size_t initial_capacity, pq_type_t type, 
                       int (*compare)(const void *a, const void *b, void *arg), 
                       void *compare_arg);

// 销毁优先队列
extern void pq_destroy(pq_t *pq);

// 清空优先队列
extern void pq_clear(pq_t *pq);

// 入队 (添加元素)
extern int pq_push(pq_t *pq, void *data);

// 出队 (删除堆顶元素并返回)
extern void* pq_pop(pq_t *pq);

// 查看堆顶元素
extern void* pq_top(const pq_t *pq);

// 获取队列大小
extern size_t pq_size(const pq_t *pq);

// 判断队列是否为空
extern int pq_empty(const pq_t *pq);

// 调整队列的容量
extern int pq_reserve(pq_t *pq, size_t new_capacity);

// 内部函数: 上浮操作
extern void pq_sift_up(pq_t *pq, size_t index);

// 内部函数: 下沉操作
extern void pq_sift_down(pq_t *pq, size_t index);

#endif // __PRIORITY_QUEUE_H__