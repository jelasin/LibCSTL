#include "list.h"
#include <stdio.h>

struct example {
    // 虽然这对于链表来说并不是必须的, 但还是为了可读性, 请将list_node 定义为第一个成员变量
    list_node list;
    int value;
};

int main(int argc, char const *argv[])
{
    // 初始化链表头
    list_head head;
    init_list_head(&head);
    // 初始化链表元素
    struct example e1, e2, e3;
    e1.value = 1;
    e2.value = 2;
    e3.value = 3;
    fprintf(stdout, "list_add_head: %d %d %d\n", e1.value, e2.value, e3.value);
    // 链表头插入元素
    list_add_head(&e1.list, &head);
    list_add_head(&e2.list, &head);
    list_add_head(&e3.list, &head);

    struct example e4, e5, e6;
    e4.value = 4;
    e5.value = 5;
    e6.value = 6;
    fprintf(stdout, "list_add_tail: %d %d %d\n", e4.value, e5.value, e6.value);
    // 链表尾插入元素
    list_add_tail(&e4.list, &head);
    list_add_tail(&e5.list, &head);
    list_add_tail(&e6.list, &head);
    // 遍历链表
    struct example *pos;
    list_for_each_entry(pos, &head, list)
    {
        fprintf(stdout, "value: %d\n", pos->value);
    }
    fprintf(stdout, "list_del %d\n", e2.value);
    // 删除链表元素
    list_del(&e2.list);
    // 遍历链表
    list_for_each_entry(pos, &head, list)
    {
        fprintf(stdout, "value: %d\n", pos->value);
    }

    fprintf(stdout, "list_add_prev list_add_next: in = %d cur = 5\n", e2.value);
    // 遍历添加节点
    struct example *next, *cur;
    list_for_each_entry_safe(pos, next, &head, list)
    {
        if (pos->value == 5)
        {
            cur = pos;
            break;
        }
    }
    struct example e7, e8;
    e7.value = 7;
    e8.value = 8;
    fprintf(stdout, "prev in = %d cur = %d\n", e7.value, cur->value);
    // // 插入节点到当前节点之前
    list_add_prev(&e7.list, &cur->list);
    fprintf(stdout, "next in = %d cur = %d\n", e8.value, cur->value);
    // // 插入节点到当前节点之后
    list_add_next(&e8.list, &cur->list);

    // 遍历链表
    list_for_each_entry(pos, &head, list)
    {
        fprintf(stdout, "value: %d\n", pos->value);
    }
    fprintf(stdout, "list_del all\n");
    // 遍历删除链表
    list_for_each_entry_safe(pos, next, &head, list)
    {
        fprintf(stdout, "del value: %d\n", pos->value);
        list_del(&pos->list);
        // 如果需要, 在这里释放内存
        // free(pos);
    }
    if (list_empty(&head))
    {
        fprintf(stdout, "list is empty\n");
    }

    return 0;
}

// ➜  list git:(master) ✗ ./exp 
// list_add_head: 1 2 3
// list_add_tail: 4 5 6
// value: 3
// value: 2
// value: 1
// value: 4
// value: 5
// value: 6
// list_del 2
// value: 3
// value: 1
// value: 4
// value: 5
// value: 6
// list_add_prev list_add_next: in = 2 cur = 5
// prev in = 7 cur = 5
// next in = 8 cur = 5
// value: 3
// value: 1
// value: 4
// value: 7
// value: 5
// value: 8
// value: 6
// list_del all
// del value: 3
// del value: 1
// del value: 4
// del value: 7
// del value: 5
// del value: 8
// del value: 6
// list is empty