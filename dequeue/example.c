#include "dequeue.h"
#include <stdio.h>

struct example {
    // 为了简化操作，请将dequeue_node作为第一个成员变量
    dequeue_node node;
    int data;
};

int main(int argc, char const *argv[])
{
    dequeue_head dq;
    dequeue_init_head(&dq);

    // 测试push操作
    struct example e1, e2, e3, e4;
    e1.data = 1;
    e2.data = 2;
    e3.data = 3;
    e4.data = 4;

    printf("Push: 1(back), 2(front), 3(back), 4(front)\n");
    dequeue_push_back(&e1.node, &dq);   // 1
    dequeue_push_front(&e2.node, &dq);  // 2 1
    dequeue_push_back(&e3.node, &dq);   // 2 1 3
    dequeue_push_front(&e4.node, &dq);  // 4 2 1 3

    // 测试front和back操作
    struct example *front = (struct example*)dequeue_front(&dq);
    struct example *back = (struct example*)dequeue_back(&dq);
    printf("Front: %d, Back: %d\n", front->data, back->data);

    // 测试pop操作
    printf("Pop front and back alternately:\n");
    struct example *e;
    
    e = (struct example*)dequeue_pop_front(&dq);
    printf("Pop front: %d\n", e->data);
    
    e = (struct example*)dequeue_pop_back(&dq);
    printf("Pop back: %d\n", e->data);
    
    e = (struct example*)dequeue_pop_front(&dq);
    printf("Pop front: %d\n", e->data);
    
    e = (struct example*)dequeue_pop_back(&dq);
    printf("Pop back: %d\n", e->data);
    
    if (dequeue_empty(&dq)) {
        printf("Dequeue is empty now\n");
    }

    return 0;
}

// 预期输出：
// Push: 1(back), 2(front), 3(back), 4(front)
// Front: 4, Back: 3
// Pop front and back alternately:
// Pop front: 4
// Pop back: 3
// Pop front: 2
// Pop back: 1
// Dequeue is empty now