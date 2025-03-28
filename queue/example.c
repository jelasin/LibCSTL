#include "queue.h"
#include <stdio.h>

struct example {
    // 将其命名在第一个位置，我们需要进行强制转换来获取结构体的地址
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

    queue_enqueue(&e1.node, &q);
    queue_enqueue(&e2.node, &q);
    queue_enqueue(&e3.node, &q);

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