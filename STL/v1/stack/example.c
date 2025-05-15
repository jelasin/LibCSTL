#include "stack.h"
#include <stdio.h>

struct example
{
    // 为了简化操作, 请将stack_node作为第一个成员变量
    stack_node node;
    int data;
};

int main(int argc, char const *argv[])
{
    stack_head head;
    stack_init_head(&head);

    struct example e1, e2, e3;
    e1.data = 1;
    e2.data = 2;
    e3.data = 3;

    stack_push(&e1.node, &head);
    stack_push(&e2.node, &head);
    stack_push(&e3.node, &head);

    
    struct example *e;
    e = (typeof(e))stack_top(&head);
    printf("top: %d\n", e->data);
    while(!stack_empty(&head))
    {
        e = (typeof(e))stack_pop(&head);
        printf("pop: %d\n", e->data);
    }
    return 0;
}

// ➜  stack git:(master) ✗ ./exp
// 3
// 2
// 1