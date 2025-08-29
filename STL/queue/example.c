#include "queue.h"
#include <stdio.h>

struct example {
    // 为了简化操作, 请将queue_node定义为第一个成员变量
    queue_node node;
    int data;
};

int main(int argc, char const *argv[])
{
    queue_head q;
    queue_init_head(&q);

    struct example e1, e2, e3;
    e1.data = 1;
    e2.data = 2;
    e3.data = 3;

    queue_enqueue(&q, &e1.node);
    queue_enqueue(&q, &e2.node);
    queue_enqueue(&q, &e3.node);

    struct example *e;
    while ((e = (typeof(e))queue_dequeue(&q)) != NULL) {
        printf("data: %d\n", e->data);
        // do free or other operations here
    }

    return 0;
}

// ➜  queue git:(master) ✗ ./exp 
// data: 1
// data: 2
// data: 3
// queue is empty: Success