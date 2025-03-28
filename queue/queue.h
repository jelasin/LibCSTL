#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "list.h"

#ifndef offsetof
// 获取结构体成员偏移，因为常量指针的值为0，即可以看作结构体首地址为0
#define offsetof(TYPE,MEMBER)((size_t)&((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
/*ptr 成员指针
* type 结构体 比如struct Stu
* member 成员变量，跟指针对应
* */
// 最后一句的意义就是，取结构体某个成员member的地址，减去这个成员在结构体type中的偏移，运算结果就是结构体type的首地址
#define container_of(ptr, type, member) ({          \
        const typeof( ((type *)0)->member ) *__mptr = (const typeof( ((type *)0)->member ) *)(ptr); \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

#ifndef list_entry
// 获取结构体指针的成员变量地址
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)
#endif

#ifndef queue_entry
// 获取队列节点的结构体指针
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
extern void queue_enqueue(queue_node *node, queue_head *head);
// 出队
extern void* queue_dequeue(queue_head *head);
// 判断队列是否为空
extern int queue_is_empty(const queue_head *head);

#endif /* __QUEUE_H__ */