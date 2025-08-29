#ifndef __RING_QUEUE_H__
#define __RING_QUEUE_H__

#include <stdlib.h>

// 环形队列状态
typedef enum {
    RING_QUEUE_SUCCESS = 0,     // 操作成功
    RING_QUEUE_EMPTY = 1,       // 队列为空
    RING_QUEUE_FULL = 2,        // 队列已满
    RING_QUEUE_ERROR = -1       // 一般错误
} ring_queue_status_t;

// 环形队列结构体
typedef struct {
    void **buffer;              // 存储元素的数组
    size_t capacity;            // 队列的容量
    size_t head;                // 队首索引
    size_t tail;                // 队尾索引
    size_t size;                // 当前元素数量
    int is_full;                // 队列是否已满的标志
    
    // 可选的析构函数，用于在出队、清空或销毁队列时释放元素
    void (*element_destructor)(void *element);
} ring_queue_t;

// 创建环形队列
extern ring_queue_t* ring_queue_create(size_t capacity, void (*element_destructor)(void *element));

// 销毁环形队列
extern void ring_queue_destroy(ring_queue_t *queue);

// 清空环形队列
extern void ring_queue_clear(ring_queue_t *queue);

// 入队操作
extern ring_queue_status_t ring_queue_enqueue(ring_queue_t *queue, void *element);

// 出队操作 (会调用析构函数)
extern ring_queue_status_t ring_queue_dequeue(ring_queue_t *queue);

// 查看队首元素但不出队
extern ring_queue_status_t ring_queue_peek(const ring_queue_t *queue, void **element);

// 判断队列是否为空
extern int ring_queue_is_empty(const ring_queue_t *queue);

// 判断队列是否已满
extern int ring_queue_is_full(const ring_queue_t *queue);

// 获取队列当前元素数量
extern size_t ring_queue_size(const ring_queue_t *queue);

// 获取队列容量
extern size_t ring_queue_capacity(const ring_queue_t *queue);

// 调整队列容量
extern ring_queue_status_t ring_queue_resize(ring_queue_t *queue, size_t new_capacity);

// 设置内存分配函数
extern void ring_queue_set_memory_alloc(void* (*alloc)(size_t size));

// 设置内存释放函数
extern void ring_queue_set_memory_free(void (*release)(void* ptr));

#endif /* __RING_QUEUE_H__ */