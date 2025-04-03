#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 定义测试结构体
struct person {
    int id;
    char name[50];
};

int main()
{
    struct person p1, p2, p3;
    p1.id = 1;
    strcpy(p1.name, "Alice");
    p2.id = 2;
    strcpy(p2.name, "Bob");
    p3.id = 3;
    strcpy(p3.name, "Charlie");

    // 创建栈
    stack_t stack;
    stack_init(&stack);

    // 压栈
    printf("压栈: %d %s, %d %s, %d %s\n", p1.id, p1.name, p2.id, p2.name, p3.id, p3.name);
    stack.push(&stack, &p1);
    stack.push(&stack, &p2);
    stack.push(&stack, &p3);

    // 栈大小
    printf("栈大小: %ld\n", stack.size);

    // 栈顶元素
    struct person *top = (struct person*)stack.top(&stack);
    printf("Top element: id=%d, name=%s\n", top->id, top->name);


    while (!stack.empty(&stack))  // 遍历栈
    {
        // 出栈
        struct person *popped = (struct person*)stack.pop(&stack);
        printf("Popped element: id=%d, name=%s\n", popped->id, popped->name);  
    }
    
    return 0;
}