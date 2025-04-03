#ifndef __STACK_H__
#define __STACK_H__

#include "list.h"
#include <stdlib.h>

// 栈节点结构体
typedef struct _stack_node {
    struct list_head list;  // 链表节点
    void *data;             // 指向用户数据的指针
} stack_node_t;

// 栈结构体
typedef struct _stack {
    struct list_head list;      // 链表头
    size_t size;                // 栈大小
    
    // 方法函数指针
    void* (*push)(struct _stack *stack, void *data);
    void* (*pop)(struct _stack *stack);
    void* (*top)(struct _stack *stack);
    int (*empty)(struct _stack *stack);
} stack_t;

// 初始化栈
extern void stack_init(stack_t *stack);

// 遍历栈中的元素（从栈顶到栈底）
#define stack_for_each_entry(pos, stack, type, member) \
    list_for_each_entry(pos, &(stack)->list, member)

#endif /* __STACK_H__ */