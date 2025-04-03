#include "rb_tree.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// 示例结构：整数节点
struct int_node {
    int value;              // 整数值
    struct rb_node node;    // 红黑树节点
};

// 整数比较函数
int compare_int(const struct rb_node *a, const struct rb_node *b, void *arg) 
{
    struct int_node *node_a = rb_entry(a, struct int_node, node);
    struct int_node *node_b = rb_entry(b, struct int_node, node);
    
    // 返回两个整数的差值
    return node_a->value - node_b->value;
}

// 整数节点析构函数
void int_destructor(struct rb_node *node, void *arg)
{
    struct int_node *int_node = rb_entry(node, struct int_node, node);
    // 如果节点是动态分配的，这里可以释放它
    // free(int_node); 
    // 注：在本例中，节点是栈上分配的，不需要释放
}

// 打印红黑树结构（用于调试）
void print_tree_structure(struct rb_node *node, int level, char prefix) 
{
    if (!node) return;
    
    // 打印缩进
    for (int i = 0; i < level; i++)
        printf("    ");
    
    // 获取节点值并打印
    struct int_node *inode = rb_entry(node, struct int_node, node);
    printf("%c[%d] (%s)\n", prefix, inode->value, 
           node->color == RB_RED ? "红" : "黑");
    
    // 递归打印左右子树
    print_tree_structure(node->left, level + 1, 'L');
    print_tree_structure(node->right, level + 1, 'R');
}

// 示例结构：字符串节点
struct string_node {
    char *str;              // 字符串
    struct rb_node node;    // 红黑树节点
};

// 字符串比较函数
int compare_string(const struct rb_node *a, const struct rb_node *b, void *arg) 
{
    struct string_node *node_a = rb_entry(a, struct string_node, node);
    struct string_node *node_b = rb_entry(b, struct string_node, node);
    
    return strcmp(node_a->str, node_b->str);
}

// 字符串节点析构函数 - 释放字符串内存
void string_destructor(struct rb_node *node, void *arg)
{
    struct string_node *str_node = rb_entry(node, struct string_node, node);
    if (str_node->str) {
        free(str_node->str);
        str_node->str = NULL;
    }
    // 注意：此示例中，节点本身是栈上分配的，不需要释放
}

// 测试整数红黑树
void test_int_tree() 
{
    rb_root_t tree;
    rb_init(&tree, compare_int, NULL, int_destructor, NULL);
    
    // 创建整数节点
    struct int_node nodes[10];
    int values[] = {50, 30, 70, 20, 40, 60, 80, 15, 25, 35};
    
    printf("插入顺序: ");
    for (int i = 0; i < 10; i++) {
        nodes[i].value = values[i];
        printf("%d ", nodes[i].value);
        rb_insert(&tree, &nodes[i].node);
    }
    printf("\n\n");
    
    // 打印树结构
    printf("树结构:\n");
    print_tree_structure(tree.root, 0, 'R');
    printf("\n");
    
    // 验证红黑树合法性
    if (rb_verify(&tree))
        printf("红黑树是合法的 ✓\n\n");
    else
        printf("红黑树不合法 ✗\n\n");
    
    // 中序遍历（有序输出）
    printf("有序遍历: ");
    struct int_node *pos;
    rb_inorder(pos, &tree, struct int_node, node) {
        printf("%d ", pos->value);
    }
    printf("\n\n");
    
    // 查找测试
    printf("查找测试:\n");
    struct int_node key;
    key.value = 40;
    struct rb_node *found = rb_search(&tree, &key.node);
    
    if (found) {
        struct int_node *found_node = rb_entry(found, struct int_node, node);
        printf("找到值 %d (颜色=%s)\n", 
               found_node->value, found->color == RB_RED ? "红" : "黑");
    } else {
        printf("未找到值 %d\n", key.value);
    }
    printf("\n");
    
    // 删除测试 - 删除叶子节点
    printf("删除叶子节点 (15):\n");
    key.value = 15;
    found = rb_search(&tree, &key.node);
    if (found) {
        rb_erase(&tree, found);
        printf("删除后的树结构:\n");
        print_tree_structure(tree.root, 0, 'R');
        
        if (rb_verify(&tree))
            printf("红黑树是合法的 ✓\n");
        else
            printf("红黑树不合法 ✗\n");
    }
    printf("\n");
    
    // 删除测试 - 删除有一个子节点的节点
    printf("删除有一个子节点的节点 (30):\n");
    key.value = 30;
    found = rb_search(&tree, &key.node);
    if (found) {
        rb_erase(&tree, found);
        printf("删除后的树结构:\n");
        print_tree_structure(tree.root, 0, 'R');
        
        if (rb_verify(&tree))
            printf("红黑树是合法的 ✓\n");
        else
            printf("红黑树不合法 ✗\n");
    }
    printf("\n");
    
    // 删除测试 - 删除有两个子节点的节点
    printf("删除有两个子节点的节点 (50):\n");
    key.value = 50;
    found = rb_search(&tree, &key.node);
    if (found) {
        rb_erase(&tree, found);
        printf("删除后的树结构:\n");
        print_tree_structure(tree.root, 0, 'R');
        
        if (rb_verify(&tree))
            printf("红黑树是合法的 ✓\n");
        else
            printf("红黑树不合法 ✗\n");
    }
    printf("\n");
    
    // 测试最小值和最大值
    struct rb_node *min_node = rb_first(&tree);
    struct rb_node *max_node = rb_last(&tree);
    
    if (min_node && max_node) {
        printf("最小值: %d\n", rb_entry(min_node, struct int_node, node)->value);
        printf("最大值: %d\n", rb_entry(max_node, struct int_node, node)->value);
    }
}

// 测试字符串红黑树
void test_string_tree() 
{
    rb_root_t tree;
    rb_init(&tree, compare_string, NULL, string_destructor, NULL);
    
    // 创建字符串节点
    struct string_node nodes[5];
    const char *strings[] = {"apple", "banana", "cherry", "date", "elderberry"};
    
    printf("\n===== 字符串树测试 =====\n");
    printf("插入顺序: ");
    for (int i = 0; i < 5; i++) {
        // 为字符串分配内存，析构函数将负责释放它
        nodes[i].str = strdup(strings[i]);
        printf("%s ", nodes[i].str);
        rb_insert(&tree, &nodes[i].node);
    }
    printf("\n\n");
    
    // 中序遍历（有序输出）
    printf("有序遍历: ");
    struct string_node *pos;
    rb_inorder(pos, &tree, struct string_node, node) {
        printf("%s ", pos->str);
    }
    printf("\n\n");
    
    // 查找测试
    printf("查找测试:\n");
    struct string_node key;
    key.str = "cherry";
    struct rb_node *found = rb_search(&tree, &key.node);
    
    if (found) {
        struct string_node *found_node = rb_entry(found, struct string_node, node);
        printf("找到字符串 '%s'\n", found_node->str);
    } else {
        printf("未找到字符串 '%s'\n", key.str);
    }
    
    // 销毁树 - 这将调用析构函数释放所有字符串
    printf("\n清理树并释放所有字符串内存\n");
    rb_destroy(&tree);
}

// 性能测试
void performance_test() 
{
    printf("\n===== 性能测试 =====\n");
    
    // 准备随机数据
    const int test_size = 100000;
    struct int_node *nodes = malloc(test_size * sizeof(struct int_node));
    
    if (!nodes) {
        printf("内存分配失败\n");
        return;
    }
    
    // 生成随机数据
    srand(time(NULL));
    for (int i = 0; i < test_size; i++) {
        nodes[i].value = rand() % 1000000;
    }
    
    // 创建红黑树 - 不使用析构函数，因为我们会手动管理内存
    rb_root_t tree;
    rb_init(&tree, compare_int, NULL, NULL, NULL);
    
    // 插入性能测试
    clock_t start = clock();
    
    for (int i = 0; i < test_size; i++) {
        rb_insert(&tree, &nodes[i].node);
    }
    
    clock_t end = clock();
    double insert_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("插入 %d 个节点: %f 秒\n", test_size, insert_time);
    
    // 验证树合法性
    printf("红黑树合法性: %s\n", rb_verify(&tree) ? "合法 ✓" : "不合法 ✗");
    
    // 查找性能测试
    start = clock();
    
    for (int i = 0; i < 10000; i++) {
        int index = rand() % test_size;
        struct int_node key;
        key.value = nodes[index].value;
        rb_search(&tree, &key.node);
    }
    
    end = clock();
    double search_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("执行 10000 次随机查找: %f 秒\n", search_time);
    
    // 删除性能测试
    start = clock();
    
    for (int i = 0; i < 10000; i++) {
        int index = rand() % test_size;
        if (nodes[index].node.parent) {  // 确保节点还在树中
            rb_erase(&tree, &nodes[index].node);
        }
    }
    
    end = clock();
    double delete_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("执行 10000 次随机删除: %f 秒\n", delete_time);
    
    // 清理
    free(nodes);
    rb_clear(&tree);  // 只清空树，不释放节点内存
}
// 析构函数，释放整个节点结构
void heap_node_destructor(struct rb_node *node, void *arg) {
    struct int_node *int_node = rb_entry(node, struct int_node, node);
    free(int_node);  // 释放整个节点
}
// 堆分配节点的示例
void test_heap_allocated_nodes()
{
    printf("\n===== 堆分配节点测试 =====\n");
    

    
    // 创建红黑树
    rb_root_t tree;
    rb_init(&tree, compare_int, NULL, heap_node_destructor, NULL);
    
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
        rb_insert(&tree, &node->node);
    }
    printf("\n");
    
    // 遍历树
    printf("有序遍历: ");
    struct int_node *pos;
    rb_inorder(pos, &tree, struct int_node, node) {
        printf("%d ", pos->value);
    }
    printf("\n");
    
    // 销毁树，这将自动释放所有节点内存
    printf("销毁树并释放所有节点内存\n");
    rb_destroy(&tree);
}

int main() 
{
    printf("===== 整数红黑树测试 =====\n");
    test_int_tree();
    
    test_string_tree();
    
    performance_test();
    
    test_heap_allocated_nodes();
    
    return 0;
}