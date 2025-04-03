#include "stack.h"
#include <stdlib.h>

// 内部入栈实现
static void* __stack_push(stack_t *stack, void *data)
{
    if (!stack) return NULL;
    
    stack_node_t *node = (stack_node_t*)malloc(sizeof(stack_node_t));
    if (!node) return NULL;
    
    node->data = data;
    list_add_head(&node->list, &stack->list);
    stack->size++;
    
    return data;
}

// 内部出栈实现
static void* __stack_pop(stack_t *stack)
{
    if (!stack || stack->empty(stack)) {
        return NULL;
    }
    
    struct list_head *first = stack->list.next;
    list_del(first);
    
    stack_node_t *node = list_entry(first, stack_node_t, list);
    void *data = node->data;
    
    free(node);  // 只释放节点，不释放用户数据
    node = NULL;
    stack->size--;
    
    return data;
}

// 内部栈顶元素获取实现
static void* __stack_top(stack_t *stack)
{
    if (!stack || stack->empty(stack)) {
        return NULL;
    }
    
    stack_node_t *node = list_entry(stack->list.next, stack_node_t, list);
    return node->data;
}

// 内部空栈检查实现
static int __stack_empty(stack_t *stack)
{
    return stack->size == 0;
}

// 初始化栈
void stack_init(stack_t *stack)
{
    if (!stack) return;
    
    init_list_head(&stack->list);
    stack->size = 0;
    
    // 设置方法函数指针
    stack->push = __stack_push;
    stack->pop = __stack_pop;
    stack->top = __stack_top;
    stack->empty = __stack_empty;
}