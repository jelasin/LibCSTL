#include "list.h"
#include <stdio.h>

struct example
{
    int value;
    struct list_head list;
};

int main(int argc, char const *argv[])
{
    // 初始化链表头
    struct list_head head;
    init_list_head(&head);
    // 初始化链表元素
    struct example e1, e2, e3;
    e1.value = 1;
    e2.value = 2;
    e3.value = 3;
    // 链表头插入元素
    list_add(&e1.list, &head);
    list_add(&e2.list, &head);
    list_add(&e3.list, &head);
    // 遍历链表
    struct example *pos;
    list_for_each_entry(pos, &head, list)
    {
        printf("value: %d\n", pos->value);
    }
    // 删除链表元素
    list_del(&e2.list);
    // 遍历链表
    list_for_each_entry(pos, &head, list)
    {
        printf("value: %d\n", pos->value);
    }
    // 链表尾插入元素
    list_add_tail(&e2.list, &head);
    // 遍历删除链表
    struct example *next;
    list_for_each_entry_safe(pos, next, &head, list)
    {
        printf("value: %d\n", pos->value);
        list_del(&pos->list);
        // 如果需要, 在这里释放内存
        // free(pos);
    }
    if (list_empty(&head))
    {
        printf("list is empty\n");
    }

    return 0;
}

// ➜  list git:(master) ✗ gcc example.c list.c -o list 
// ➜  list git:(master) ✗ ./list            
// value: 3
// value: 2
// value: 1
// value: 3
// value: 1
// value: 3
// value: 1
// value: 2
// list is empty