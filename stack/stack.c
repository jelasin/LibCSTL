#include "stack.h"

// 初始化栈头
void stack_init_head(stack_head *head)
{
    head->list.next = head->list.prev = &head->list;
}
// 入栈操作
void stack_push(stack_node *node, stack_head *head)
{
    list_add_head(&node->list, &head->list);
}
// 出栈操作
void *stack_pop(stack_head *head)
{
    if (stack_empty(head))
    {
        return (void*)0;
    }
    stack_node *node = stack_entry((stack_node*)head->list.next, stack_node, list);
    list_del(&node->list);

    return (void*)node;
}
// 判断栈是否为空
int stack_empty(const stack_head *head)
{
    return head->list.next == &head->list;
}