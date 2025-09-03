#include "avl_tree.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

static char *xstrdup(const char *s) {
    size_t n = strlen(s);
    char *p = (char *)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}

// 示例结构：整数节点
struct int_node {
    int value;                 // 值
    struct avl_node node;      // AVL树节点
};

// 整数比较函数
int compare_int(const struct avl_node *a, const struct avl_node *b, void *arg) 
{
    (void)arg; // 未使用的比较额外参数
    struct int_node *node_a = avl_entry(a, struct int_node, node);
    struct int_node *node_b = avl_entry(b, struct int_node, node);
    
    return node_a->value - node_b->value;
}

// 整数节点析构函数
void int_destructor(struct avl_node *node, void *arg)
{
    (void)arg; // 未使用的析构额外参数
    struct int_node *int_node = avl_entry(node, struct int_node, node);
    (void)int_node; // 示例中未实际释放，避免未使用变量告警
    // 如果节点是动态分配的，这里可以释放它
    // free(int_node); 
    // 注：在本例中，节点是栈上分配的，不需要释放
}

// 示例结构：字符串节点
struct string_node {
    char *str;                 // 字符串
    struct avl_node node;      // AVL树节点
};

// 字符串比较函数
int compare_string(const struct avl_node *a, const struct avl_node *b, void *arg) 
{
    (void)arg; // 未使用的比较额外参数
    struct string_node *node_a = avl_entry(a, struct string_node, node);
    struct string_node *node_b = avl_entry(b, struct string_node, node);
    
    return strcmp(node_a->str, node_b->str);
}

// 字符串节点析构函数
void string_destructor(struct avl_node *node, void *arg)
{
    (void)arg; // 未使用的析构额外参数
    struct string_node *str_node = avl_entry(node, struct string_node, node);
    if (str_node->str) {
        // 对于从堆上分配的字符串，需要释放内存
        free(str_node->str);
        str_node->str = NULL;
    }
    // 注：此示例中节点本身是栈上分配的，所以不需要释放
}

// 验证AVL树是否平衡
int validate_avl_tree(struct avl_node *node) 
{
    if (!node) return 1;
    
    // 检查左右子树是否平衡
    if (!validate_avl_tree(node->left) || !validate_avl_tree(node->right))
        return 0;
    
    // 检查当前节点的平衡因子
    int balance = avl_balance_factor(node);
    if (balance < -1 || balance > 1)
        return 0;
    
    // 检查高度是否正确
    int left_height = avl_height(node->left);
    int right_height = avl_height(node->right);
    int expected_height = (left_height > right_height ? left_height : right_height) + 1;
    
    if (node->height != expected_height)
        return 0;
    
    return 1;
}

// 打印AVL树结构（用于调试）
void print_tree_structure(struct avl_node *node, int level, char prefix) 
{
    if (!node) return;
    
    for (int i = 0; i < level; i++)
        printf("    ");
    
    struct int_node *inode = avl_entry(node, struct int_node, node);
    printf("%c[%d] (h=%d, bf=%d)\n", prefix, inode->value, 
           node->height, avl_balance_factor(node));
    
    print_tree_structure(node->left, level + 1, 'L');
    print_tree_structure(node->right, level + 1, 'R');
}

void test_int_tree() 
{
    avl_root_t tree;
    avl_init(&tree, compare_int, NULL, int_destructor, NULL);
    
    // 创建整数节点 - 使用不平衡的插入顺序
    struct int_node nodes[10];
    int values[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    printf("插入顺序: ");
    for (int i = 0; i < 10; i++) {
        nodes[i].value = values[i];
        printf("%d ", nodes[i].value);
        avl_insert(&tree, &nodes[i].node);
    }
    printf("\n\n");
    
    // 打印树结构
    printf("树结构:\n");
    print_tree_structure(tree.root, 0, 'R');
    printf("\n");
    
    // 验证树是否平衡
    if (validate_avl_tree(tree.root))
        printf("树是平衡的 ✓\n\n");
    else
        printf("树不平衡 ✗\n\n");
    
    // 中序遍历（有序输出）
    printf("有序遍历: ");
    struct int_node *pos;
    avl_inorder(pos, &tree, struct int_node, node) {
        printf("%d ", pos->value);
    }
    printf("\n\n");
    
    // 查找测试
    printf("查找测试:\n");
    struct int_node key;
    key.value = 40;
    struct avl_node *found = avl_search(&tree, &key.node);
    
    if (found) {
        struct int_node *found_node = avl_entry(found, struct int_node, node);
        printf("找到值 %d (高度=%d, 平衡因子=%d)\n", 
               found_node->value, found->height, avl_balance_factor(found));
    } else {
        printf("未找到值 %d\n", key.value);
    }
    printf("\n");
    
    // 删除测试 - 删除叶子节点
    printf("删除叶子节点 (100):\n");
    key.value = 100;
    found = avl_search(&tree, &key.node);
    if (found) {
        avl_erase(&tree, found);
        printf("删除后的树结构:\n");
        print_tree_structure(tree.root, 0, 'R');
        
        if (validate_avl_tree(tree.root))
            printf("树是平衡的 ✓\n");
        else
            printf("树不平衡 ✗\n");
    }
    printf("\n");
    
    // 删除测试 - 删除有一个子节点的节点
    printf("删除有一个子节点的节点 (30):\n");
    key.value = 30;
    found = avl_search(&tree, &key.node);
    if (found) {
        avl_erase(&tree, found);
        printf("删除后的树结构:\n");
        print_tree_structure(tree.root, 0, 'R');
        
        if (validate_avl_tree(tree.root))
            printf("树是平衡的 ✓\n");
        else
            printf("树不平衡 ✗\n");
    }
    printf("\n");
    
    // 删除测试 - 删除有两个子节点的节点
    printf("删除有两个子节点的节点 (40):\n");
    key.value = 40;
    found = avl_search(&tree, &key.node);
    if (found) {
        avl_erase(&tree, found);
        printf("删除后的树结构:\n");
        print_tree_structure(tree.root, 0, 'R');
        
        if (validate_avl_tree(tree.root))
            printf("树是平衡的 ✓\n");
        else
            printf("树不平衡 ✗\n");
    }
    printf("\n");
    
    // 测试最小值和最大值
    struct avl_node *min_node = avl_first(&tree);
    struct avl_node *max_node = avl_last(&tree);
    
    if (min_node && max_node) {
        printf("最小值: %d\n", avl_entry(min_node, struct int_node, node)->value);
        printf("最大值: %d\n", avl_entry(max_node, struct int_node, node)->value);
    }
}

void test_string_tree() 
{
    avl_root_t tree;
    avl_init(&tree, compare_string, NULL, string_destructor, NULL);
    
    // 创建字符串节点
    struct string_node nodes[5];
    const char *strings[] = {"apple", "banana", "cherry", "date", "elderberry"};
    
    printf("\n===== 字符串树测试 =====\n");
    printf("插入顺序: ");
    for (int i = 0; i < 5; i++) {
        // 为字符串分配内存，析构函数将负责释放它
        nodes[i].str = xstrdup(strings[i]);
        printf("%s ", nodes[i].str);
        avl_insert(&tree, &nodes[i].node);
    }
    printf("\n\n");
    
    // 中序遍历（有序输出）
    printf("有序遍历: ");
    struct string_node *pos;
    avl_inorder(pos, &tree, struct string_node, node) {
        printf("%s ", pos->str);
    }
    printf("\n\n");
    
    // 查找测试
    printf("查找测试:\n");
    struct string_node key;
    key.str = "cherry";
    struct avl_node *found = avl_search(&tree, &key.node);
    
    if (found) {
        struct string_node *found_node = avl_entry(found, struct string_node, node);
        printf("找到字符串 '%s'\n", found_node->str);
    } else {
        printf("未找到字符串 '%s'\n", key.str);
    }
    
    // 销毁树 - 这将调用析构函数释放所有字符串
    printf("\n清理树并释放所有字符串内存\n");
    avl_destroy(&tree);
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
        nodes[i].value = rand() % 100000;
    }
    
    // 创建AVL树 - 修复：添加缺少的参数
    avl_root_t tree;
    avl_init(&tree, compare_int, NULL, NULL, NULL);
    
    // 插入性能测试
    clock_t start = clock();
    
    for (int i = 0; i < test_size; i++) {
        avl_insert(&tree, &nodes[i].node);
    }
    
    clock_t end = clock();
    double insert_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("插入 %d 个节点: %f 秒\n", test_size, insert_time);
    
    // 验证树是否平衡
    if (validate_avl_tree(tree.root))
        printf("树是平衡的 ✓\n");
    else
        printf("树不平衡 ✗\n");
    
    // 查找性能测试
    start = clock();
    
    for (int i = 0; i < 100000; i++) {
        int index = rand() % test_size;
        struct int_node key;
        key.value = nodes[index].value;
        avl_search(&tree, &key.node);
    }
    
    end = clock();
    double search_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("执行 100000 次随机查找: %f 秒\n", search_time);
    
    // 清理
    // 销毁树（未提供析构函数时，仅断开树结构，不释放nodes数组元素内存）
    avl_destroy(&tree);
    free(nodes);
}
// 析构函数，释放整个节点结构
void heap_node_destructor(struct avl_node *node, void *arg) {
    (void)arg; // 未使用的析构额外参数
    struct int_node *int_node = avl_entry(node, struct int_node, node);
    free(int_node);  // 释放整个节点
}
// 堆分配节点测试
void test_heap_allocated_nodes()
{
    printf("\n===== 堆分配节点测试 =====\n");
    

    
    // 创建AVL树
    avl_root_t tree;
    avl_init(&tree, compare_int, NULL, heap_node_destructor, NULL);
    
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
        avl_insert(&tree, &node->node);
    }
    printf("\n");
    
    // 遍历树
    printf("有序遍历: ");
    struct int_node *pos;
    avl_inorder(pos, &tree, struct int_node, node) {
        printf("%d ", pos->value);
    }
    printf("\n");
    
    // 销毁树并释放所有节点内存
    printf("销毁树并释放所有节点内存\n");
    avl_destroy(&tree);
}

int main() 
{
    test_int_tree();
    test_string_tree();
    performance_test();
    test_heap_allocated_nodes();
    
    return 0;
}

// 预期输出示例:
// 插入顺序: 10 20 30 40 50 60 70 80 90 100 
// 
// 树结构:
// R[40] (h=4, bf=0)
//     L[20] (h=3, bf=0)
//         L[10] (h=1, bf=0)
//         R[30] (h=1, bf=0)
//     R[70] (h=3, bf=0)
//         L[50] (h=2, bf=0)
//             L[40] (h=1, bf=0)
//             R[60] (h=1, bf=0)
//         R[90] (h=2, bf=0)
//             L[80] (h=1, bf=0)
//             R[100] (h=1, bf=0)
// 
// 树是平衡的 ✓
// 
// 有序遍历: 10 20 30 40 50 60 70 80 90 100 
// 
// 查找测试:
// 找到值 40 (高度=4, 平衡因子=0)
// 
// 删除叶子节点 (100):
// 删除后的树结构:
// R[40] (h=4, bf=0)
//     L[20] (h=3, bf=0)
//         L[10] (h=1, bf=0)
//         R[30] (h=1, bf=0)
//     R[70] (h=3, bf=0)
//         L[50] (h=2, bf=0)
//             L[40] (h=1, bf=0)
//             R[60] (h=1, bf=0)
//         R[90] (h=1, bf=1)
//             L[80] (h=1, bf=0)
// 树是平衡的 ✓
// 
// ...