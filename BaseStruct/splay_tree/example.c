#include "splay_tree.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // 添加stdlib.h以使用strdup和free

// 示例结构：整数节点
struct int_node {
    struct splay_node node;  // 伸展树节点
    int value;
};

// 整数比较函数
int compare_int(const struct splay_node *a, const struct splay_node *b, void *arg) 
{
    struct int_node *node_a = splay_entry(a, struct int_node, node);
    struct int_node *node_b = splay_entry(b, struct int_node, node);
    
    return node_a->value - node_b->value;
}

// 整数节点析构函数
void int_destructor(struct splay_node *node, void *arg)
{
    struct int_node *int_node = splay_entry(node, struct int_node, node);
    // 在本例中节点是栈上分配的，不需要释放
    // 但如果是动态分配的，可以在这里释放
    // free(int_node);
}

// 示例结构：字符串节点
struct string_node {
    struct splay_node node;  // 伸展树节点
    char *str;
};

// 字符串比较函数
int compare_string(const struct splay_node *a, const struct splay_node *b, void *arg) 
{
    struct string_node *node_a = splay_entry(a, struct string_node, node);
    struct string_node *node_b = splay_entry(b, struct string_node, node);
    
    return strcmp(node_a->str, node_b->str);
}

// 字符串节点析构函数
void string_destructor(struct splay_node *node, void *arg)
{
    struct string_node *str_node = splay_entry(node, struct string_node, node);
    if (str_node->str) {
        free(str_node->str);
        str_node->str = NULL;
    }
    // 注意：在本例中节点是栈上分配的，所以不需要释放节点本身
}

void test_int_tree() 
{
    splay_root_t tree;
    splay_init(&tree, compare_int, NULL, int_destructor, NULL);
    
    // 创建整数节点
    struct int_node nodes[10];
    int values[] = {5, 3, 7, 2, 4, 6, 8, 1, 9, 0};
    
    printf("插入顺序: ");
    for (int i = 0; i < 10; i++) {
        nodes[i].value = values[i];
        printf("%d ", nodes[i].value);
        splay_insert(&tree, &nodes[i].node);
    }
    printf("\n");
    
    // 中序遍历（有序输出）
    printf("有序遍历: ");
    struct int_node *pos;
    splay_inorder(pos, &tree, struct int_node, node) {
        printf("%d ", pos->value);
    }
    printf("\n");
    
    // 查找测试
    printf("\n查找测试:\n");
    struct int_node key;
    key.value = 4;
    struct splay_node *found = splay_search(&tree, &key.node);
    
    if (found) {
        struct int_node *found_node = splay_entry(found, struct int_node, node);
        printf("找到值 %d\n", found_node->value);
        printf("根节点现在是 %d\n", ((struct int_node*)splay_entry(tree.root, struct int_node, node))->value);
    } else {
        printf("未找到值 %d\n", key.value);
    }
    
    // 删除测试
    printf("\n删除测试:\n");
    key.value = 5;
    found = splay_search(&tree, &key.node);
    
    if (found) {
        struct int_node *found_node = splay_entry(found, struct int_node, node);
        printf("删除值 %d\n", found_node->value);
        splay_erase(&tree, found);
        
        // 再次遍历
        printf("删除后遍历: ");
        splay_inorder(pos, &tree, struct int_node, node) {
            printf("%d ", pos->value);
        }
        printf("\n");
    }
    
    // 最小值和最大值测试
    struct splay_node *min_node = splay_first(&tree);
    struct splay_node *max_node = splay_last(&tree);
    
    if (min_node && max_node) {
        printf("\n最小值: %d\n", ((struct int_node*)splay_entry(min_node, struct int_node, node))->value);
        printf("最大值: %d\n", ((struct int_node*)splay_entry(max_node, struct int_node, node))->value);
    }
}

void test_string_tree() 
{
    splay_root_t tree;
    splay_init(&tree, compare_string, NULL, string_destructor, NULL);
    
    // 创建字符串节点
    struct string_node nodes[5];
    const char *strings[] = {"apple", "banana", "orange", "grape", "kiwi"};
    
    printf("\n\n===== 字符串树测试 =====\n");
    printf("插入顺序: ");
    for (int i = 0; i < 5; i++) {
        // 使用strdup分配内存，析构函数将负责释放它
        nodes[i].str = strdup(strings[i]);
        printf("%s ", nodes[i].str);
        splay_insert(&tree, &nodes[i].node);
    }
    printf("\n");
    
    // 中序遍历（有序输出）
    printf("有序遍历: ");
    struct string_node *pos;
    splay_inorder(pos, &tree, struct string_node, node) {
        printf("%s ", pos->str);
    }
    printf("\n");
    
    // 查找测试
    printf("\n查找测试:\n");
    struct string_node key;
    key.str = "orange";
    struct splay_node *found = splay_search(&tree, &key.node);
    
    if (found) {
        struct string_node *found_node = splay_entry(found, struct string_node, node);
        printf("找到字符串 '%s'\n", found_node->str);
        printf("根节点现在是 '%s'\n", ((struct string_node*)splay_entry(tree.root, struct string_node, node))->str);
    }
    
    // 测试结束后销毁树，这将调用析构函数释放所有字符串
    printf("\n销毁树并释放所有字符串内存\n");
    splay_destroy(&tree);
}
// 析构函数，释放整个节点结构
void heap_node_destructor(struct splay_node *node, void *arg) {
    struct int_node *int_node = splay_entry(node, struct int_node, node);
    free(int_node); // 释放整个节点结构
}
// 堆分配节点测试
void test_heap_allocated_nodes()
{
    printf("\n===== 堆分配节点测试 =====\n");
    

    // 创建伸展树
    splay_root_t tree;
    splay_init(&tree, compare_int, NULL, heap_node_destructor, NULL);
    
    // 添加一些堆分配的节点
    printf("添加堆分配的节点: ");
    for (int i = 0; i < 10; i++) {
        struct int_node *node = malloc(sizeof(struct int_node));
        if (!node) {
            printf("内存分配失败\n");
            break;
        }
        node->value = i * 10;
        printf("%d ", node->value);
        splay_insert(&tree, &node->node);
    }
    printf("\n");
    
    // 遍历树
    printf("有序遍历: ");
    struct int_node *pos;
    splay_inorder(pos, &tree, struct int_node, node) {
        printf("%d ", pos->value);
    }
    printf("\n");
    
    // 销毁树，这将自动释放所有节点内存
    printf("销毁树并释放所有节点内存\n");
    splay_destroy(&tree);
}

int main() 
{
    printf("===== 整数树测试 =====\n");
    test_int_tree();
    
    test_string_tree();
    
    test_heap_allocated_nodes();
    
    return 0;
}

// 预期输出:
// ===== 整数树测试 =====
// 插入顺序: 5 3 7 2 4 6 8 1 9 0 
// 有序遍历: 0 1 2 3 4 5 6 7 8 9 
// 
// 查找测试:
// 找到值 4
// 根节点现在是 4
// 
// 删除测试:
// 删除值 5
// 删除后遍历: 0 1 2 3 4 6 7 8 9 
// 
// 最小值: 0
// 最大值: 9
// 
// ===== 字符串树测试 =====
// 插入顺序: apple banana orange grape kiwi 
// 有序遍历: apple banana grape kiwi orange 
// 
// 查找测试:
// 找到字符串 'orange'
// 根节点现在是 'orange'