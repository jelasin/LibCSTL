#include "ring_queue.h"

#ifdef DEBUG
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>

typedef void* (*malloc_func)(size_t size);
typedef void (*free_func)(void* ptr);

static malloc_func malloc_hook = malloc;
static free_func free_hook = free;

// 设置内存分配函数
void ring_queue_set_memory_alloc(void* (*alloc)(size_t size))
{
    malloc_hook = alloc;
}

// 设置内存释放函数
void ring_queue_set_memory_free(void (*release)(void* ptr))
{
    free_hook = release;
}

// 创建环形队列
ring_queue_t* ring_queue_create(size_t capacity, void (*element_destructor)(void *element))
{
    if (capacity == 0) {
#ifdef DEBUG
        perror("[ring_queue_create] Ring queue capacity must be greater than zero.");
#endif
        return NULL;
    }
    
    ring_queue_t *queue = (ring_queue_t*)malloc_hook(sizeof(ring_queue_t));
    if (!queue) {
#ifdef DEBUG
        perror("[ring_queue_create] Failed to allocate memory for ring queue.");
#endif
        return NULL;
    }
    
    queue->buffer = (void**)malloc_hook(capacity * sizeof(void*));
    if (!queue->buffer) {
#ifdef DEBUG
        perror("[ring_queue_create] Failed to allocate memory for ring queue buffer.");
#endif
        free_hook(queue);
        return NULL;
    }
    
    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    queue->is_full = 0;
    queue->element_destructor = element_destructor;
    
    return queue;
}

// 销毁环形队列
void ring_queue_destroy(ring_queue_t *queue)
{
    if (!queue) {
#ifdef DEBUG
        perror("[ring_queue_destroy] Queue is NULL.");
#endif
        return;
    }
    
    // 清空队列，会调用析构函数
    ring_queue_clear(queue);
    
    // 释放缓冲区
    free_hook(queue->buffer);
    queue->buffer = NULL;
    
    // 释放队列结构
    free_hook(queue);
}

// 清空环形队列
void ring_queue_clear(ring_queue_t *queue)
{
    if (!queue) {
#ifdef DEBUG
        perror("[ring_queue_clear] Queue is NULL.");
#endif
        return;
    }
    
    // 如果有析构函数，则对每个元素调用它
    if (queue->element_destructor) {
        while (!ring_queue_is_empty(queue)) {
            if (ring_queue_dequeue(queue) == RING_QUEUE_ERROR) {
#ifdef DEBUG
                perror("[ring_queue_clear] Failed to dequeue element.");
#endif
                break;
            }
        }
    } else {
        // 没有析构函数，直接重置队列状态
        queue->head = 0;
        queue->tail = 0;
        queue->size = 0;
        queue->is_full = 0;
    }
}

// 入队操作
ring_queue_status_t ring_queue_enqueue(ring_queue_t *queue, void *element)
{
    if (!queue || !element) {
#ifdef DEBUG
        perror("[ring_queue_enqueue] Queue or element is NULL.");
#endif
        return RING_QUEUE_ERROR;
    }
    
    // 检查队列是否已满
    if (ring_queue_is_full(queue)) {
#ifdef DEBUG
        perror("[ring_queue_enqueue] Ring queue is full.");
#endif
        return RING_QUEUE_FULL;
    }
    
    // 添加元素到队尾
    queue->buffer[queue->tail] = element;
    
    // 更新队尾索引
    queue->tail = (queue->tail + 1) % queue->capacity;
    
    // 更新队列大小
    queue->size++;
    
    // 检查队列是否已满
    if (queue->head == queue->tail) {
        queue->is_full = 1;
    }
    
    return RING_QUEUE_SUCCESS;
}

// 出队操作 (会调用析构函数)
ring_queue_status_t ring_queue_dequeue(ring_queue_t *queue)
{
    if (!queue) {
#ifdef DEBUG
        perror("[ring_queue_dequeue] Queue is NULL.");
#endif
        return RING_QUEUE_ERROR;
    }

    // 检查队列是否为空
    if (ring_queue_is_empty(queue)) {
#ifdef DEBUG
        perror("[ring_queue_dequeue] Ring queue is empty.");
#endif
        return RING_QUEUE_EMPTY;
    }
    
    // 获取队首元素（使用普通指针，不是指针的指针）
    void *element = queue->buffer[queue->head];
    
    // 更新队首索引
    queue->head = (queue->head + 1) % queue->capacity;
    
    // 更新队列大小和状态
    queue->size--;
    queue->is_full = 0;
    
    // 如果有析构函数且数据不为空，调用析构函数
    if (queue->element_destructor && element) {
        queue->element_destructor(element);
    }
    
    return RING_QUEUE_SUCCESS;
}

// 查看队首元素但不出队
ring_queue_status_t ring_queue_peek(const ring_queue_t *queue, void **element)
{
    if (!queue || !element) {
#ifdef DEBUG
        perror("[ring_queue_peek] Queue or element is NULL.");
#endif
        return RING_QUEUE_ERROR;
    }
    
    // 检查队列是否为空
    if (ring_queue_is_empty(queue)) {
#ifdef DEBUG
        perror("[ring_queue_peek] Ring queue is empty.");
#endif
        *element = NULL;
        return RING_QUEUE_EMPTY;
    }
    
    // 返回队首元素
    *element = queue->buffer[queue->head];
    
    return RING_QUEUE_SUCCESS;
}

// 判断队列是否为空
int ring_queue_is_empty(const ring_queue_t *queue)
{
    if (!queue) {
        return 1;  // 视为空队列
    }
    
    return (queue->head == queue->tail && !queue->is_full);
}

// 判断队列是否已满
int ring_queue_is_full(const ring_queue_t *queue)
{
    if (!queue) {
        return 0;  // 视为非满队列
    }
    
    return queue->is_full;
}

// 获取队列当前元素数量
size_t ring_queue_size(const ring_queue_t *queue)
{
    if (!queue) {
        return 0;
    }
    
    return queue->size;
}

// 获取队列容量
size_t ring_queue_capacity(const ring_queue_t *queue)
{
    if (!queue) {
        return 0;
    }
    
    return queue->capacity;
}

// 调整队列容量
ring_queue_status_t ring_queue_resize(ring_queue_t *queue, size_t new_capacity)
{
    if (!queue || new_capacity == 0) {
#ifdef DEBUG
        perror("[ring_queue_resize] Queue is NULL or new capacity is zero.");
#endif
        return RING_QUEUE_ERROR;
    }
    
    // 如果新容量小于当前元素数量，则失败
    if (new_capacity < queue->size) {
#ifdef DEBUG
        perror("[ring_queue_resize] New capacity is less than current size.");
#endif
        return RING_QUEUE_ERROR;
    }
    
    // 分配新的缓冲区
    void **new_buffer = (void**)malloc_hook(new_capacity * sizeof(void*));
    if (!new_buffer) {
#ifdef DEBUG
        perror("[ring_queue_resize] Failed to allocate memory for new buffer.");
#endif
        return RING_QUEUE_ERROR;
    }
    
    // 拷贝旧缓冲区的元素到新缓冲区
    size_t j = 0;
    
    // 处理环绕的情况
    if (queue->tail < queue->head || queue->is_full) {
        // 复制从head到buffer末尾的元素
        for (size_t i = queue->head; i < queue->capacity; i++) {
            new_buffer[j++] = queue->buffer[i];
        }
        
        // 复制从buffer开头到tail的元素
        for (size_t i = 0; i < queue->tail; i++) {
            new_buffer[j++] = queue->buffer[i];
        }
    } else {
        // 没有环绕，直接复制
        for (size_t i = queue->head; i < queue->tail; i++) {
            new_buffer[j++] = queue->buffer[i];
        }
    }
    
    // 释放旧缓冲区
    free_hook(queue->buffer);
    
    // 更新队列状态
    queue->buffer = new_buffer;
    queue->capacity = new_capacity;
    queue->head = 0;
    queue->tail = j;
    queue->is_full = (j == new_capacity);
    
    return RING_QUEUE_SUCCESS;
}