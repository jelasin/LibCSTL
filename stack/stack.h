#ifndef __STACK_H__
#define __STACK_H__

#include "list.h"

#ifndef stack_entry
typedef unsigned long size_t;
#define stack_entry(ptr, type, member) \
    container_of(ptr, type, member)
#endif

struct stack_list  {
    list_head list;
};

typedef struct stack_list stack_head;
typedef struct stack_list stack_node;
// 初始化栈头
extern void stack_init_head(stack_head *head);
// 入栈操作
extern void stack_push(stack_node *node, stack_head *head);
// 出栈操作
extern void* stack_pop(stack_head *head);
// 判断栈是否为空
extern int stack_empty(const stack_head *head);
// 查看栈顶元素
extern void* stack_top(const stack_head *head);


#endif /* __STACK_H__ */