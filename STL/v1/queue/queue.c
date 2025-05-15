#include "queue.h"

// 初始化队列头
void queue_init_head(queue_head *head)
{
    head->list.next = head->list.prev = &head->list;
}
// 入队
void queue_enqueue(queue_head *head, queue_node *node)
{
    list_add_head(&node->list, &head->list);
}
// 出队
void* queue_dequeue(queue_head *head)
{
    if (queue_is_empty(head))
    {
        return (void*)0;
    }

    queue_node *node = queue_entry((queue_node*)head->list.prev, queue_node, list);
    list_del(&node->list);

    return (void*)node;
}
// 查看下一个出队元素
void* queue_peek(const queue_head *head)
{
    if (queue_is_empty(head))
    {
        return (void*)0;
    }

    queue_node *node = queue_entry((queue_node*)head->list.prev, queue_node, list);
    return (void*)node;
}
// 判断队列是否为空
int queue_is_empty(const queue_head *head)
{
    return head->list.next == &head->list;
}