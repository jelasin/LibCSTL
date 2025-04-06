#include "ring_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 用于测试析构功能的字符串析构函数
void string_destructor(void *element)
{
    if (element) {
        printf("销毁字符串: %s\n", (char*)element);
        free(element);
    }
}

// 整数析构函数
void int_destructor(void *element)
{
    if (element) {
        printf("销毁整数: %d\n", *(int*)element);
        free(element);
    }
}

// 基本操作演示函数
void basic_operations_demo()
{
    printf("\n===== 基本操作演示 =====\n");
    
    // 创建一个带析构函数的环形队列
    ring_queue_t *queue = ring_queue_create(5, int_destructor);
    if (!queue) {
        printf("创建队列失败\n");
        return;
    }
    
    int values[] = {10, 20, 30, 40, 50, 60};
    
    // 入队操作
    printf("入队元素: ");
    for (int i = 0; i < 6; i++) {
        int *val = malloc(sizeof(int));
        *val = values[i];
        
        ring_queue_status_t status = ring_queue_enqueue(queue, val);
        if (status == RING_QUEUE_SUCCESS) {
            printf("%d ", *val);
        } else if (status == RING_QUEUE_FULL) {
            printf("\n队列已满，无法继续入队 %d\n", *val);
            free(val);  // 入队失败需要手动释放内存
        }
    }
    printf("\n");
    
    // 查看队列大小和容量
    printf("队列大小: %zu/%zu\n", ring_queue_size(queue), ring_queue_capacity(queue));
    
    // 查看队首元素
    void *element;
    if (ring_queue_peek(queue, &element) == RING_QUEUE_SUCCESS) {
        printf("队首元素: %d\n", *(int*)element);
    }
    
    // 使用会调用析构函数的出队方法
    printf("\n使用 dequeue 出队剩余元素 (自动调用析构函数):\n");
    while (!ring_queue_is_empty(queue)) {
        ring_queue_dequeue(queue);  // element将被置为NULL
    }
    
    // 尝试从空队列出队
    ring_queue_status_t status = ring_queue_dequeue(queue);
    printf("\n尝试从空队列出队: %s\n", 
           status == RING_QUEUE_EMPTY ? "队列为空" : "未知错误");
    
    // 销毁队列
    ring_queue_destroy(queue);
}

// 析构函数演示
void auto_destructor_demo()
{
    printf("\n===== 析构函数演示 =====\n");
    
    // 创建一个带析构函数的环形队列
    ring_queue_t *queue = ring_queue_create(5, string_destructor);
    if (!queue) {
        printf("创建队列失败\n");
        return;
    }
    
    const char *strings[] = {"Hello", "World", "Ring", "Queue", "Demo"};
    
    // 入队操作
    printf("入队字符串元素:\n");
    for (int i = 0; i < 5; i++) {
        char *str = strdup(strings[i]);  // 在堆上复制字符串
        printf("  添加: %s\n", str);
        ring_queue_enqueue(queue, str);
    }
    
    // 查看队首元素而不出队
    void *element;
    if (ring_queue_peek(queue, &element) == RING_QUEUE_SUCCESS) {
        printf("\n队首元素: %s (未出队)\n", (char*)element);
    }
    
    // 出队部分元素 - 调用析构函数
    printf("\n出队部分元素 (自动调用析构函数):\n");
    for (int i = 0; i < 2; i++) {
        ring_queue_dequeue(queue);  // element被析构后置为NULL
    }
    
    // 剩余元素通过清空函数自动析构
    printf("\n清空队列 (自动调用析构函数):\n");
    ring_queue_clear(queue);
    
    printf("\n添加更多元素并销毁队列:\n");
    for (int i = 0; i < 2; i++) {
        char *str = strdup(strings[i]);
        printf("  添加: %s\n", str);
        ring_queue_enqueue(queue, str);
    }
    
    // 销毁队列 - 调用析构函数
    printf("\n销毁队列 (自动调用析构函数):\n");
    ring_queue_destroy(queue);
}

// 调整大小功能演示
void resize_demo()
{
    printf("\n===== 调整大小功能演示 =====\n");
    
    // 创建一个容量为3的环形队列，带析构函数
    ring_queue_t *queue = ring_queue_create(3, int_destructor);
    if (!queue) {
        printf("创建队列失败\n");
        return;
    }
    
    // 填充队列
    printf("初始队列大小: %zu/%zu\n", ring_queue_size(queue), ring_queue_capacity(queue));
    printf("填充队列...\n");
    
    for (int i = 1; i <= 3; i++) {
        int *val = malloc(sizeof(int));
        *val = i * 10;
        ring_queue_enqueue(queue, val);
    }
    
    printf("填充后队列大小: %zu/%zu\n", ring_queue_size(queue), ring_queue_capacity(queue));
    
    // 扩大队列容量
    printf("\n扩大队列容量到6...\n");
    if (ring_queue_resize(queue, 6) == RING_QUEUE_SUCCESS) {
        printf("调整后队列大小: %zu/%zu\n", ring_queue_size(queue), ring_queue_capacity(queue));
        
        // 添加更多元素
        printf("添加更多元素...\n");
        for (int i = 4; i <= 6; i++) {
            int *val = malloc(sizeof(int));
            *val = i * 10;
            ring_queue_enqueue(queue, val);
        }
        
        printf("添加后队列大小: %zu/%zu\n", ring_queue_size(queue), ring_queue_capacity(queue));
    } else {
        printf("调整队列容量失败\n");
    }
    
    // 使用析构函数自动释放所有元素
    printf("\n调用 destroy 销毁队列 (自动调用析构函数):\n");
    ring_queue_destroy(queue);
}

int main()
{
    basic_operations_demo();
    auto_destructor_demo();
    resize_demo();
    
    return 0;
}