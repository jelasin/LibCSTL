#include "lru_list.h"
#include <stdio.h>
#include <string.h>

// 示例：简单的字符串缓存
struct string_cache_entry {
    struct lru_node node;       // LRU节点
    char *key;                  // 键
    char *value;                // 值
};

// 字符串哈希函数
unsigned int string_hash(const void *key, void *arg) 
{
    const char *str = (const char*)key;
    unsigned int hash = 0;
    
    // 简单的字符串哈希
    while (*str) {
        hash = hash * 31 + (*str++);
    }
    
    return hash;
}

// 字符串比较函数
int string_compare(const void *a, const void *b, void *arg) 
{
    const char *key = (const char*)a;
    struct string_cache_entry *entry = lru_entry(b, struct string_cache_entry, node);
    return strcmp(key, entry->key);
}

int main() 
{
    // 创建LRU链表，最大容量为5，哈希桶为16
    lru_list_t *cache = lru_create(5, 16, string_hash, string_compare, NULL);
    
    // 创建一些缓存条目
    struct string_cache_entry entries[7];
    
    // 初始化条目
    entries[0].key = "apple";
    entries[0].value = "红色水果";
    
    entries[1].key = "banana";
    entries[1].value = "黄色水果";
    
    entries[2].key = "cherry";
    entries[2].value = "红色小果";
    
    entries[3].key = "date";
    entries[3].value = "棕色果实";
    
    entries[4].key = "elderberry";
    entries[4].value = "紫色小果";
    
    entries[5].key = "fig";
    entries[5].value = "绿色果实";
    
    entries[6].key = "grape";
    entries[6].value = "紫色水果";
    
    printf("=== 添加条目 (最大容量为5) ===\n");
    
    // 添加前5个条目（未超出最大容量）
    for (int i = 0; i < 5; i++) {
        lru_put(cache, &entries[i].node, entries[i].key);
        printf("添加: %s -> %s\n", entries[i].key, entries[i].value);
    }
    
    printf("\nLRU链表大小: %u\n", lru_size(cache));
    
    printf("\n=== 遍历LRU链表 (从最近使用到最少使用) ===\n");
    struct string_cache_entry *pos;
    lru_for_each(pos, cache, struct string_cache_entry, node) {
        printf("%s -> %s\n", pos->key, pos->value);
    }
    
    printf("\n=== 访问 'apple' ===\n");
    struct lru_node *apple_node = lru_get(cache, "apple");
    if (apple_node) {
        struct string_cache_entry *apple = lru_entry(apple_node, struct string_cache_entry, node);
        printf("找到: %s -> %s\n", apple->key, apple->value);
    }
    
    printf("\n=== 再次遍历LRU链表 (apple现在应该在最前面) ===\n");
    lru_for_each(pos, cache, struct string_cache_entry, node) {
        printf("%s -> %s\n", pos->key, pos->value);
    }
    
    printf("\n=== 添加超过容量的条目 ===\n");
    // 添加第6个条目（超出最大容量）
    printf("添加: %s -> %s\n", entries[5].key, entries[5].value);
    lru_put(cache, &entries[5].node, entries[5].key);
    
    printf("\n=== 最近最少使用的条目应该被移除 ===\n");
    lru_for_each(pos, cache, struct string_cache_entry, node) {
        printf("%s -> %s\n", pos->key, pos->value);
    }
    
    printf("\n=== 添加另一个条目 ===\n");
    // 添加第7个条目
    printf("添加: %s -> %s\n", entries[6].key, entries[6].value);
    lru_put(cache, &entries[6].node, entries[6].key);
    
    printf("\n=== 最终LRU链表 ===\n");
    lru_for_each(pos, cache, struct string_cache_entry, node) {
        printf("%s -> %s\n", pos->key, pos->value);
    }
    
    printf("\n=== 检查存在性 ===\n");
    printf("'apple'存在? %s\n", lru_contains(cache, "apple") ? "是" : "否");
    printf("'banana'存在? %s\n", lru_contains(cache, "banana") ? "是" : "否");  // 应该已被移除
    
    printf("\n=== 移除条目 ===\n");
    printf("移除 'apple': %s\n", lru_remove(cache, "apple") == 0 ? "成功" : "失败");
    
    printf("\n=== 移除后的LRU链表 ===\n");
    lru_for_each(pos, cache, struct string_cache_entry, node) {
        printf("%s -> %s\n", pos->key, pos->value);
    }
    
    printf("\n=== 清空缓存 ===\n");
    lru_clear(cache);
    printf("LRU链表为空? %s\n", lru_empty(cache) ? "是" : "否");
    
    // 销毁LRU链表
    lru_destroy(cache);
    
    return 0;
}

// 预期输出:
// === 添加条目 (最大容量为5) ===
// 添加: apple -> 红色水果
// 添加: banana -> 黄色水果
// 添加: cherry -> 红色小果
// 添加: date -> 棕色果实
// 添加: elderberry -> 紫色小果
// 
// LRU链表大小: 5
// 
// === 遍历LRU链表 (从最近使用到最少使用) ===
// elderberry -> 紫色小果
// date -> 棕色果实
// cherry -> 红色小果
// banana -> 黄色水果
// apple -> 红色水果
// 
// === 访问 'apple' ===
// 找到: apple -> 红色水果
// 
// === 再次遍历LRU链表 (apple现在应该在最前面) ===
// apple -> 红色水果
// elderberry -> 紫色小果
// date -> 棕色果实
// cherry -> 红色小果
// banana -> 黄色水果
// 
// === 添加超过容量的条目 ===
// 添加: fig -> 绿色果实
// 
// === 最近最少使用的条目应该被移除 ===
// fig -> 绿色果实
// apple -> 红色水果
// elderberry -> 紫色小果
// date -> 棕色果实
// cherry -> 红色小果
// 
// === 添加另一个条目 ===
// 添加: grape -> 紫色水果
// 
// === 最终LRU链表 ===
// grape -> 紫色水果
// fig -> 绿色果实
// apple -> 红色水果
// elderberry -> 紫色小果
// date -> 棕色果实
// 
// === 检查存在性 ===
// 'apple'存在? 是
// 'banana'存在? 否
// 
// === 移除条目 ===
// 移除 'apple': 成功
// 
// === 移除后的LRU链表 ===
// grape -> 紫色水果
// fig -> 绿色果实
// elderberry -> 紫色小果
// date -> 棕色果实
// 
// === 清空缓存 ===
// LRU链表为空? 是