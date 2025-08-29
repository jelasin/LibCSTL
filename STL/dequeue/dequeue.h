#ifndef __DEQUEUE_H__
#define __DEQUEUE_H__

#include "list.h"

#ifndef dequeue_entry
typedef unsigned long size_t;
#define dequeue_entry(ptr, type, member) \
    container_of(ptr, type, member)
#endif

struct dequeue_list {
    struct list_head list;
};

typedef struct dequeue_list dequeue_head;
typedef struct dequeue_list dequeue_node;

// 初始化双端队列头
extern void dequeue_init_head(dequeue_head *head);

// 从队列前端入队
extern void dequeue_push_front(dequeue_node *node, dequeue_head *head);

// 从队列后端入队
extern void dequeue_push_back(dequeue_node *node, dequeue_head *head);

// 从队列前端出队
extern void* dequeue_pop_front(dequeue_head *head);

// 从队列后端出队
extern void* dequeue_pop_back(dequeue_head *head);

// 获取队列前端元素
extern void* dequeue_front(const dequeue_head *head);

// 获取队列后端元素
extern void* dequeue_back(const dequeue_head *head);

// 判断队列是否为空
extern int dequeue_empty(const dequeue_head *head);

#endif // __DEQUEUE_H__