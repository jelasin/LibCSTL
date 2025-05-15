#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "list.h"

#ifndef queue_entry
// 获取队列节点的结构体指针
typedef unsigned long size_t;
#define queue_entry(ptr, type, member) \
    container_of(ptr, type, member)
#endif

struct queue_list {
    struct list_head list;
};
typedef struct queue_list queue_head;
typedef struct queue_list queue_node;

// 初始化队列头
extern void queue_init_head(queue_head *head);
// 入队
extern void queue_enqueue(queue_head *head, queue_node *node);
// 出队
extern void* queue_dequeue(queue_head *head);
// 查看下一个出队元素
extern void* queue_peek(const queue_head *head);
// 判断队列是否为空
extern int queue_is_empty(const queue_head *head);

#endif /* __QUEUE_H__ */