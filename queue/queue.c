#include "queue.h"
#include <stdio.h>

// 初始化队列头
void queue_init_head(queue_head *head)
{
    head->list.next = head->list.prev = &head->list;
}
// 入队
void queue_enqueue(queue_node *node, queue_head *head)
{
    list_add_head(&node->list, &head->list);
}
// 出队
void* queue_dequeue(queue_head *head)
{
    if (queue_is_empty(head))
    {
        perror("queue is empty");
        return NULL;
    }
    queue_node *node = queue_entry((queue_node*)head->list.prev, queue_node, list);
    list_del(&node->list);

    return (void*)node;
}
// 判断队列是否为空
int queue_is_empty(const queue_head *head)
{
    return head->list.next == &head->list;
}