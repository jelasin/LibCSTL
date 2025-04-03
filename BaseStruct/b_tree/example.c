#include "b_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

// 整数比较函数
int compare_int(const void *a, const void *b, void *arg) {
    int int_a = *(int*)a;
    int int_b = *(int*)b;
    
    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

// 整数析构函数
void int_destructor(void *key, void *arg) {
    free(key);
}

// 整数打印回调函数
void print_int(void *key, void *arg) {
    printf("%d ", *(int*)key);
}

// 字符串比较函数
int compare_string(const void *a, const void *b, void *arg) {
    return strcmp((const char*)a, (const char*)b);
}

// 字符串析构函数
void string_destructor(void *key, void *arg) {
    free(key);
}

// 字符串打印回调函数
void print_string(void *key, void *arg) {
    printf("'%s' ", (char*)key);
}

// 测试整数B树
void test_int_btree() {
    printf("===== 整数B树测试 =====\n");
    
    // 创建B树，阶为5
    btree_t *tree = btree_create(5, compare_int, NULL, int_destructor, NULL);
    
    // 插入一些整数
    int values[] = {50, 30, 70, 20, 40, 60, 80, 15, 25, 35, 45, 55, 65, 75, 85};
    int count = sizeof(values) / sizeof(values[0]);
    
    printf("插入顺序: ");
    for (int i = 0; i < count; i++) {
        int *val = (int*)malloc(sizeof(int));
        *val = values[i];
        printf("%d ", *val);
        btree_insert(tree, val);
    }
    printf("\n\n");
    
    // 打印B树属性
    printf("B树高度: %d\n", btree_height(tree));
    printf("关键字数量: %d\n\n", btree_count(tree));
    
    // 中序遍历（有序输出）
    printf("有序遍历: ");
    btree_inorder(tree, print_int, NULL);
    printf("\n\n");
    
    // 查找测试
    int search_val = 40;
    void *result = btree_search(tree, &search_val);
    if (result) {
        printf("找到值: %d\n", *(int*)result);
    } else {
        printf("未找到值: %d\n", search_val);
    }
    
    // 最小值和最大值
    void *min = btree_get_min(tree);
    void *max = btree_get_max(tree);
    if (min && max) {
        printf("最小值: %d\n", *(int*)min);
        printf("最大值: %d\n\n", *(int*)max);
    }
    
    // 删除测试
    int del_val = 50;
    printf("删除值: %d\n", del_val);
    btree_delete(tree, &del_val);
    
    printf("删除后的有序遍历: ");
    btree_inorder(tree, print_int, NULL);
    printf("\n");
    printf("删除后B树高度: %d\n", btree_height(tree));
    printf("删除后关键字数量: %d\n\n", btree_count(tree));
    
    // 清空B树
    printf("清空B树\n");
    btree_clear(tree);
    printf("B树为空? %s\n\n", btree_empty(tree) ? "是" : "否");
    
    // 销毁B树
    btree_destroy(tree);
}

// 测试字符串B树
void test_string_btree() {
    printf("===== 字符串B树测试 =====\n");
    
    // 创建B树，阶为4
    btree_t *tree = btree_create(4, compare_string, NULL, string_destructor, NULL);
    
    // 插入一些字符串
    const char *strings[] = {"apple", "banana", "cherry", "date", "elderberry", "fig", "grape", "honeydew"};
    int count = sizeof(strings) / sizeof(strings[0]);
    
    printf("插入顺序: ");
    for (int i = 0; i < count; i++) {
        char *str = strdup(strings[i]);
        printf("%s ", str);
        btree_insert(tree, str);
    }
    printf("\n\n");
    
    // 打印B树属性
    printf("B树高度: %d\n", btree_height(tree));
    printf("关键字数量: %d\n\n", btree_count(tree));
    
    // 中序遍历（有序输出）
    printf("有序遍历: ");
    btree_inorder(tree, print_string, NULL);
    printf("\n\n");
    
    // 查找测试
    const char *search_str = "cherry";
    void *result = btree_search(tree, search_str);
    if (result) {
        printf("找到字符串: '%s'\n", (char*)result);
    } else {
        printf("未找到字符串: '%s'\n", search_str);
    }
    
    // 最小值和最大值
    void *min = btree_get_min(tree);
    void *max = btree_get_max(tree);
    if (min && max) {
        printf("最小值: '%s'\n", (char*)min);
        printf("最大值: '%s'\n\n", (char*)max);
    }
    
    // 删除测试
    const char *del_str = "banana";
    printf("删除字符串: '%s'\n", del_str);
    btree_delete(tree, del_str);
    
    printf("删除后的有序遍历: ");
    btree_inorder(tree, print_string, NULL);
    printf("\n");
    printf("删除后B树高度: %d\n", btree_height(tree));
    printf("删除后关键字数量: %d\n\n", btree_count(tree));
    
    // 销毁B树
    btree_destroy(tree);
}

// 测试B树性能
void test_btree_performance() {
    printf("===== B树性能测试 =====\n");
    
    // 创建B树，阶为7（比较大的阶数可能会更高效）
    btree_t *tree = btree_create(7, compare_int, NULL, int_destructor, NULL);
    
    // 测试参数
    int num_elements = 10000;
    
    // 插入性能测试
    printf("插入 %d 个元素...\n", num_elements);
    clock_t start = clock();
    
    for (int i = 0; i < num_elements; i++) {
        int *val = (int*)malloc(sizeof(int));
        *val = i;
        btree_insert(tree, val);
    }
    
    clock_t end = clock();
    double insert_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("插入耗时: %.6f 秒\n", insert_time);
    
    // 树的属性
    printf("B树高度: %d\n", btree_height(tree));
    printf("关键字数量: %d\n\n", btree_count(tree));
    
    // 搜索性能测试
    printf("执行 %d 次随机搜索...\n", num_elements);
    start = clock();
    
    for (int i = 0; i < num_elements; i++) {
        int search_val = rand() % num_elements;
        btree_search(tree, &search_val);
    }
    
    end = clock();
    double search_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("搜索耗时: %.6f 秒\n\n", search_time);
    
    // 销毁B树
    btree_destroy(tree);
}

int main() {
    test_int_btree();
    test_string_btree();
    test_btree_performance();
    
    return 0;
}