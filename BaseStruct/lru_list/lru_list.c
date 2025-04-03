#include "lru_list.h"
#include <stdlib.h>

// 默认的哈希函数（简单地返回指针值的一部分）
static unsigned int default_hash_func(const void *key, void *arg) 
{
    unsigned long ptr_val = (unsigned long)key;
    unsigned int buckets = *(unsigned int*)arg;
    return (unsigned int)(ptr_val % buckets);
}

// 默认的比较函数（直接比较指针）
static int default_compare(const void *a, const void *b, void *arg) 
{
    return (a == b) ? 0 : ((a < b) ? -1 : 1);
}

// 创建LRU链表
lru_list_t* lru_create(unsigned int max_size, unsigned int buckets,
                       unsigned int (*hash_func)(const void *key, void *arg),
                       int (*compare)(const void *a, const void *b, void *arg),
                       void *user_arg) 
{
    lru_list_t *lru = (lru_list_t*)malloc(sizeof(lru_list_t));
    if (!lru) 
        return NULL;
    
    // 初始化链表
    init_list_head(&lru->list);
    
    // 分配哈希表空间
    lru->htable = (struct hlist_head*)malloc(buckets * sizeof(struct hlist_head));
    if (!lru->htable) {
        free(lru);
        return NULL;
    }
    
    // 初始化哈希表
    for (unsigned int i = 0; i < buckets; i++) {
        hlist_init_head(&lru->htable[i]);
    }
    
    lru->size = 0;
    lru->max_size = max_size;
    lru->buckets = buckets;
    
    // 设置哈希和比较函数
    lru->hash_func = hash_func ? hash_func : default_hash_func;
    lru->compare = compare ? compare : default_compare;
    lru->user_arg = user_arg ? user_arg : &lru->buckets;
    
    return lru;
}

// 销毁LRU链表
void lru_destroy(lru_list_t *lru) 
{
    if (lru) {
        free(lru->htable);
        free(lru);
    }
}

// 在哈希表中查找节点
static struct lru_node* find_node(lru_list_t *lru, const void *key) 
{
    unsigned int hash = lru->hash_func(key, lru->user_arg);
    struct hlist_head *bucket = &lru->htable[hash % lru->buckets];
    struct hlist_node *pos;
    
    hlist_for_each(pos, bucket) {
        struct lru_node *node = hlist_entry(pos, struct lru_node, hnode);
        if (lru->compare(key, node, lru->user_arg) == 0) {
            return node;
        }
    }
    
    return NULL;
}

// 添加/更新元素
int lru_put(lru_list_t *lru, struct lru_node *node, const void *key) 
{
    // 首先检查元素是否已存在
    struct lru_node *existing = find_node(lru, key);
    
    if (existing) {
        // 如果是同一个节点，只需要移动到链表头部
        if (existing == node) {
            list_del(&existing->list);
            list_add_head(&existing->list, &lru->list);
            return 0;
        }
        
        // 如果是不同节点，移除旧节点
        hlist_del(&existing->hnode);
        list_del(&existing->list);
        lru->size--;
    }
    
    // 如果达到最大容量，移除最近最少使用的元素
    if (lru->size >= lru->max_size && lru->max_size > 0) {
        struct lru_node *lru_node = lru_get_lru(lru);
        if (lru_node) {
            hlist_del(&lru_node->hnode);
            list_del(&lru_node->list);
            lru->size--;
        }
    }
    
    // 计算哈希值
    unsigned int hash = lru->hash_func(key, lru->user_arg);
    struct hlist_head *bucket = &lru->htable[hash % lru->buckets];
    
    // 添加到哈希表
    hlist_add_head(&node->hnode, bucket);
    
    // 添加到链表头部（最近使用）
    list_add_head(&node->list, &lru->list);
    
    lru->size++;
    return 0;
}

// 获取元素，同时将其移动到链表头部
struct lru_node* lru_get(lru_list_t *lru, const void *key) 
{
    struct lru_node *node = find_node(lru, key);
    
    if (node) {
        // 移动到链表头部
        list_del(&node->list);
        list_add_head(&node->list, &lru->list);
    }
    
    return node;
}

// 移除元素
int lru_remove(lru_list_t *lru, const void *key) 
{
    struct lru_node *node = find_node(lru, key);
    
    if (node) {
        hlist_del(&node->hnode);
        list_del(&node->list);
        lru->size--;
        return 0;
    }
    
    return -1;  // 未找到
}

// 检查元素是否存在
int lru_contains(lru_list_t *lru, const void *key) 
{
    return find_node(lru, key) != NULL;
}

// 获取LRU链表大小
unsigned int lru_size(const lru_list_t *lru) 
{
    return lru->size;
}

// 判断LRU链表是否为空
int lru_empty(const lru_list_t *lru) 
{
    return list_empty(&lru->list);
}

// 清空LRU链表
void lru_clear(lru_list_t *lru) 
{
    // 初始化链表
    init_list_head(&lru->list);
    
    // 初始化哈希表
    for (unsigned int i = 0; i < lru->buckets; i++) {
        hlist_init_head(&lru->htable[i]);
    }
    
    lru->size = 0;
}

// 获取最近最少使用的元素（链表尾部元素）
struct lru_node* lru_get_lru(const lru_list_t *lru) 
{
    if (lru_empty(lru))
        return NULL;
    
    return lru_entry(lru->list.prev, struct lru_node, list);
}

// 获取最近使用的元素（链表头部元素）
struct lru_node* lru_get_mru(const lru_list_t *lru) 
{
    if (lru_empty(lru))
        return NULL;
    
    return lru_entry(lru->list.next, struct lru_node, list);
}