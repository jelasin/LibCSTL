#include "hashmap.h"
#include <stdio.h>
#include <string.h>

// 用于遍历的回调函数
bool print_entry(const void *key, size_t key_size,
                void *value, size_t value_size,
                void *user_data) {
    printf("键: %s, 值: %d\n", (const char *)key, *(int *)value);
    return 1;
}

int main() {
    // 创建哈希映射
    hashmap_t *map = hashmap_create(16, 0.75f, hashmap_hash_string, hashmap_compare_string);
    if (!map) {
        printf("创建哈希映射失败\n");
        return 1;
    }
    
    // 插入一些键值对
    const char *keys[] = {"apple", "banana", "cherry", "date", "elderberry"};
    int values[] = {10, 20, 30, 40, 50};
    
    printf("插入键值对...\n");
    for (int i = 0; i < 5; i++) {
        if (hashmap_put(map, keys[i], strlen(keys[i]) + 1, &values[i], sizeof(int)) != HASHMAP_OK) {
            printf("插入键 '%s' 失败\n", keys[i]);
        }
    }
    
    // 查询键值对
    printf("\n查询键值对...\n");
    for (int i = 0; i < 5; i++) {
        int value;
        if (hashmap_get(map, keys[i], strlen(keys[i]) + 1, &value, sizeof(int), NULL) == HASHMAP_OK) {
            printf("键 '%s' 的值为: %d\n", keys[i], value);
        } else {
            printf("找不到键 '%s'\n", keys[i]);
        }
    }
    
    // 修改值
    printf("\n修改键值...\n");
    int new_value = 100;
    hashmap_put(map, "banana", strlen("banana") + 1, &new_value, sizeof(int));
    
    int value;
    hashmap_get(map, "banana", strlen("banana") + 1, &value, sizeof(int), NULL);
    printf("修改后 'banana' 的值为: %d\n", value);
    
    // 遍历哈希映射
    printf("\n遍历哈希映射...\n");
    hashmap_foreach(map, print_entry, NULL);
    
    // 检查键是否存在
    printf("\n检查键是否存在...\n");
    printf("'apple' 存在: %s\n", hashmap_contains(map, "apple", strlen("apple") + 1) ? "是" : "否");
    printf("'grape' 存在: %s\n", hashmap_contains(map, "grape", strlen("grape") + 1) ? "是" : "否");
    
    // 删除键值对
    printf("\n删除键值对...\n");
    hashmap_remove(map, "cherry", strlen("cherry") + 1);
    printf("删除后 'cherry' 存在: %s\n", hashmap_contains(map, "cherry", strlen("cherry") + 1) ? "是" : "否");
    
    // 再次遍历
    printf("\n删除后遍历...\n");
    hashmap_foreach(map, print_entry, NULL);
    
    // 清空哈希映射
    printf("\n清空哈希映射...\n");
    hashmap_clear(map);
    printf("哈希映射大小: %zu\n", hashmap_size(map));
    
    // 销毁哈希映射
    hashmap_destroy(map);
    printf("\n哈希映射已销毁\n");
    
    return 0;
}