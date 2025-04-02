#include "dequeue.h"

// 初始化双端队列头
void dequeue_init_head(dequeue_head *head)
{
    head->list.next = head->list.prev = &head->list;
}

// 从队列前端入队
void dequeue_push_front(dequeue_node *node, dequeue_head *head)
{
    list_add_head(&node->list, &head->list);
}

// 从队列后端入队
void dequeue_push_back(dequeue_node *node, dequeue_head *head)
{
    list_add_tail(&node->list, &head->list);
}

// 从队列前端出队
void* dequeue_pop_front(dequeue_head *head)
{
    if (dequeue_empty(head))
    {
        return (void*)0;
    }
    
    dequeue_node *node = dequeue_entry((dequeue_node*)head->list.next, dequeue_node, list);
    list_del(&node->list);
    
    return (void*)node;
}

// 从队列后端出队
void* dequeue_pop_back(dequeue_head *head)
{
    if (dequeue_empty(head))
    {
        return (void*)0;
    }
    
    dequeue_node *node = dequeue_entry((dequeue_node*)head->list.prev, dequeue_node, list);
    list_del(&node->list);
    
    return (void*)node;
}

// 获取队列前端元素
void* dequeue_front(const dequeue_head *head)
{
    if (dequeue_empty(head))
    {
        return (void*)0;
    }
    
    dequeue_node *node = dequeue_entry((dequeue_node*)head->list.next, dequeue_node, list);
    return (void*)node;
}

// 获取队列后端元素
void* dequeue_back(const dequeue_head *head)
{
    if (dequeue_empty(head))
    {
        return (void*)0;
    }
    
    dequeue_node *node = dequeue_entry((dequeue_node*)head->list.prev, dequeue_node, list);
    return (void*)node;
}

// 判断队列是否为空
int dequeue_empty(const dequeue_head *head)
{
    return head->list.next == &head->list;
}