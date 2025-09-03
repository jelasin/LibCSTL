#include "rb_tree.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

/*
 * Linux内核风格红黑树示例程序
 * 展示两种使用方式：
 * 1. Linux内核风格的低级API
 * 2. 兼容性的高级API
 */

// 示例结构：整数节点
struct int_node {
    int value;              // 整数值
    struct rb_node node;    // 红黑树节点
};

// 整数比较函数（用于兼容性API）
int compare_int(const struct rb_node *a, const struct rb_node *b, void *arg) 
{
    struct int_node *node_a = rb_entry(a, struct int_node, node);
    struct int_node *node_b = rb_entry(b, struct int_node, node);
    
    return node_a->value - node_b->value;
}

// 整数节点析构函数
void int_destructor(struct rb_node *node, void *arg)
{
    struct int_node *int_node = rb_entry(node, struct int_node, node);
    printf("Destroying node with value %d\n", int_node->value);
    // 在实际应用中，这里会调用 free(int_node)
}

// 打印红黑树结构（调试用）
void print_tree_structure(struct rb_node *node, int level, char prefix) 
{
    if (!node) return;
    
    for (int i = 0; i < level; i++)
        printf("    ");
    
    struct int_node *inode = rb_entry(node, struct int_node, node);
    printf("%c[%d] (%s)\n", prefix, inode->value, 
           rb_is_red(node) ? "RED" : "BLACK");
    
    print_tree_structure(node->rb_left, level + 1, 'L');
    print_tree_structure(node->rb_right, level + 1, 'R');
}

/*
 * Linux内核风格API示例
 */
void test_kernel_style_api()
{
    printf("=== Linux Kernel Style API Test ===\n");
    
    // 初始化根节点
    struct rb_root root = RB_ROOT;
    
    // 创建测试节点
    struct int_node nodes[10];
    int values[] = {50, 25, 75, 12, 37, 62, 87, 6, 18, 43};
    
    // 插入节点（Linux内核风格）
    printf("Inserting nodes: ");
    for (int i = 0; i < 10; i++) {
        nodes[i].value = values[i];
        rb_init_node(&nodes[i].node);
        
        // 手动查找插入位置
        struct rb_node **new_node = &root.rb_node;
        struct rb_node *parent = NULL;
        
        while (*new_node) {
            struct int_node *this_node = rb_entry(*new_node, struct int_node, node);
            
            parent = *new_node;
            if (nodes[i].value < this_node->value) {
                new_node = &((*new_node)->rb_left);
            } else if (nodes[i].value > this_node->value) {
                new_node = &((*new_node)->rb_right);
            } else {
                printf("Duplicate value %d\n", nodes[i].value);
                break;
            }
        }
        
        // 链接并着色
        rb_link_node(&nodes[i].node, parent, new_node);
        rb_insert_color(&nodes[i].node, &root);
        
        printf("%d ", values[i]);
    }
    printf("\n\n");
    
    // 打印树结构
    printf("Tree structure:\n");
    print_tree_structure(root.rb_node, 0, 'R');
    printf("\n");
    
    // 中序遍历
    printf("In-order traversal: ");
    for (struct rb_node *node = rb_first(&root); node; node = rb_next(node)) {
        struct int_node *data = rb_entry(node, struct int_node, node);
        printf("%d ", data->value);
    }
    printf("\n");
    
    // 逆序遍历
    printf("Reverse traversal: ");
    for (struct rb_node *node = rb_last(&root); node; node = rb_prev(node)) {
        struct int_node *data = rb_entry(node, struct int_node, node);
        printf("%d ", data->value);
    }
    printf("\n");
    
    // 查找测试
    printf("Search test:\n");
    int search_values[] = {25, 99, 6, 100};
    for (int i = 0; i < 4; i++) {
        struct rb_node *node = root.rb_node;
        struct int_node *found = NULL;
        
        while (node) {
            struct int_node *data = rb_entry(node, struct int_node, node);
            if (search_values[i] < data->value) {
                node = node->rb_left;
            } else if (search_values[i] > data->value) {
                node = node->rb_right;
            } else {
                found = data;
                break;
            }
        }
        
        printf("  Search %d: %s\n", search_values[i], 
               found ? "FOUND" : "NOT FOUND");
    }
    
    // 删除节点测试 - 直接使用数组索引，避免指针问题
    printf("\nSafe deletion test:\n");
    
    // 删除节点25 (nodes[1])
    printf("Deleting node 25 (nodes[1])...\n");
    // 使用兼容层接口删除：构造临时包装以复用 rb_erase
    rb_root_t tmp_tree; rb_init(&tmp_tree, compare_int, NULL, NULL, NULL);
    tmp_tree.root = root; /* 复制底层root */
    rb_erase(&tmp_tree, &nodes[1].node);
    root = tmp_tree.root; /* 回写变更 */
    
    printf("Tree after deleting 25:\n");
    print_tree_structure(root.rb_node, 0, 'R');
    printf("In-order: ");
    for (struct rb_node *node = rb_first(&root); node; node = rb_next(node)) {
        struct int_node *data = rb_entry(node, struct int_node, node);
        printf("%d ", data->value);
    }
    printf("\n\n");
    
    // 删除节点75 (nodes[2])
    printf("Deleting node 75 (nodes[2])...\n");
    tmp_tree.root = root;
    rb_erase(&tmp_tree, &nodes[2].node);
    root = tmp_tree.root;
    
    printf("Tree after deleting 75:\n");
    print_tree_structure(root.rb_node, 0, 'R');
    printf("Final in-order: ");
    for (struct rb_node *node = rb_first(&root); node; node = rb_next(node)) {
        struct int_node *data = rb_entry(node, struct int_node, node);
        printf("%d ", data->value);
    }
    printf("\n\n");
    
    printf("After deletion: ");
    for (struct rb_node *node = rb_first(&root); node; node = rb_next(node)) {
        struct int_node *data = rb_entry(node, struct int_node, node);
        printf("%d ", data->value);
    }
    printf("\n\n");
}

/*
 * 兼容性API示例
 */
void test_compatibility_api()
{
    printf("=== Compatibility API Test ===\n");
    
    rb_root_t tree;
    rb_init(&tree, compare_int, NULL, int_destructor, NULL);
    
    // 创建测试节点
    struct int_node *nodes = malloc(8 * sizeof(struct int_node));
    int values[] = {40, 20, 60, 10, 30, 50, 70, 35};
    
    // 插入节点
    printf("Inserting nodes: ");
    for (int i = 0; i < 8; i++) {
        nodes[i].value = values[i];
        rb_init_node(&nodes[i].node);
        
        int result = rb_insert(&tree, &nodes[i].node);
        if (result == 0) {
            printf("%d ", values[i]);
        } else {
            printf("(DUP:%d) ", values[i]);
        }
    }
    printf("\n\n");
    
    // 查找测试
    printf("Search test:\n");
    int search_values[] = {30, 80, 10, 35};
    for (int i = 0; i < 4; i++) {
        struct int_node key;
        key.value = search_values[i];
        rb_init_node(&key.node);
        
        struct rb_node *found = rb_search(&tree, &key.node);
        if (found) {
            struct int_node *data = rb_entry(found, struct int_node, node);
            printf("  Found %d at node %p\n", data->value, (void*)found);
        } else {
            printf("  Not found %d\n", search_values[i]);
        }
    }
    
    // 使用遍历宏
    printf("\nUsing traversal macro: ");
    struct int_node *pos;
    rb_inorder(pos, &tree, struct int_node, node) {
        printf("%d ", pos->value);
    }
    printf("\n");
    
    // 验证红黑树性质
    printf("Tree verification: %s\n", 
           rb_verify(&tree) ? "PASSED" : "FAILED");
    
    // 清空树（会调用析构函数）
    printf("\nClearing tree:\n");
    rb_clear(&tree);
    
    printf("Tree is empty: %s\n", 
           rb_empty(&tree) ? "YES" : "NO");
    
    free(nodes);
}

/*
 * 性能测试
 */
void test_performance()
{
    printf("=== Performance Test ===\n");
    
    const int N = 100000;
    struct rb_root root = RB_ROOT;
    struct int_node *nodes = malloc(N * sizeof(struct int_node));
    
    if (!nodes) {
        printf("Memory allocation failed\n");
        return;
    }
    
    // 随机数种子
    srand((unsigned int)time(NULL));
    
    // 插入测试
    clock_t start = clock();
    for (int i = 0; i < N; i++) {
        nodes[i].value = rand() % (N * 2);  // 允许重复以测试查找
        rb_init_node(&nodes[i].node);
        
        struct rb_node **new_node = &root.rb_node;
        struct rb_node *parent = NULL;
        
        while (*new_node) {
            struct int_node *this_node = rb_entry(*new_node, struct int_node, node);
            
            parent = *new_node;
            if (nodes[i].value < this_node->value) {
                new_node = &((*new_node)->rb_left);
            } else if (nodes[i].value > this_node->value) {
                new_node = &((*new_node)->rb_right);
            } else {
                goto skip_insert;  // 跳过重复值
            }
        }
        
        rb_link_node(&nodes[i].node, parent, new_node);
        rb_insert_color(&nodes[i].node, &root);
        
        skip_insert:;
    }
    clock_t end = clock();
    printf("Inserted %d nodes in %.3f seconds\n", 
           N, (double)(end - start) / CLOCKS_PER_SEC);
    
    // 查找测试
    int found_count = 0;
    start = clock();
    for (int i = 0; i < N / 10; i++) {
        int search_value = rand() % (N * 2);
        struct rb_node *node = root.rb_node;
        
        while (node) {
            struct int_node *data = rb_entry(node, struct int_node, node);
            if (search_value < data->value) {
                node = node->rb_left;
            } else if (search_value > data->value) {
                node = node->rb_right;
            } else {
                found_count++;
                break;
            }
        }
    }
    end = clock();
    printf("Searched %d times, found %d, took %.3f seconds\n", 
           N / 10, found_count, (double)(end - start) / CLOCKS_PER_SEC);
    
    // 遍历测试
    int count = 0;
    start = clock();
    for (struct rb_node *node = rb_first(&root); node; node = rb_next(node)) {
        count++;
    }
    end = clock();
    printf("Traversed %d nodes in %.3f seconds\n", 
           count, (double)(end - start) / CLOCKS_PER_SEC);
    
    free(nodes);
}

/*
 * 主函数
 */
int main()
{
    printf("Enhanced Red-Black Tree Implementation\n");
    printf("Based on Linux Kernel Design\n");
    printf("======================================\n\n");
    
    // 测试Linux内核风格API
    test_kernel_style_api();
    
    // 测试兼容性API
    test_compatibility_api();
    
    // 性能测试
    test_performance();
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}
