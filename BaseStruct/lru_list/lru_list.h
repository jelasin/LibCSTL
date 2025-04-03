#ifndef __LRU_LIST_H__
#define __LRU_LIST_H__

#include "list.h"
#include "hlist.h"

#ifndef lru_entry
typedef unsigned long size_t;
#define lru_entry(ptr, type, member) \
    container_of(ptr, type, member)
#endif

// LRU节点定义，包含一个哈希表节点和一个双向链表节点
struct lru_node {
    struct hlist_node hnode;    // 哈希表节点（用于快速查找）
    struct list_head list;      // 链表节点（用于维护访问顺序）
};

// LRU链表结构
typedef struct lru_list {
    struct list_head list;      // 按访问顺序排序的链表
    struct hlist_head *htable;  // 哈希表（用于O(1)查找）
    unsigned int size;          // 当前元素数量
    unsigned int max_size;      // 最大容量
    unsigned int buckets;       // 哈希桶数量
    
    // 哈希函数，返回元素的哈希值
    unsigned int (*hash_func)(const void *key, void *arg);
    
    // 比较函数，用于比较两个元素是否相同
    int (*compare)(const void *a, const void *b, void *arg);
    
    // 用户自定义参数
    void *user_arg;
} lru_list_t;

// 创建LRU链表
extern lru_list_t* lru_create(unsigned int max_size, unsigned int buckets,
                              unsigned int (*hash_func)(const void *key, void *arg),
                              int (*compare)(const void *a, const void *b, void *arg),
                              void *user_arg);

// 销毁LRU链表
extern void lru_destroy(lru_list_t *lru);

// 添加/更新元素，如果元素已存在则移动到链表头部
extern int lru_put(lru_list_t *lru, struct lru_node *node, const void *key);

// 获取元素，同时将其移动到链表头部
extern struct lru_node* lru_get(lru_list_t *lru, const void *key);

// 移除元素
extern int lru_remove(lru_list_t *lru, const void *key);

// 检查元素是否存在
extern int lru_contains(lru_list_t *lru, const void *key);

// 获取LRU链表大小
extern unsigned int lru_size(const lru_list_t *lru);

// 判断LRU链表是否为空
extern int lru_empty(const lru_list_t *lru);

// 清空LRU链表
extern void lru_clear(lru_list_t *lru);

// 获取最近最少使用的元素（链表尾部元素）
extern struct lru_node* lru_get_lru(const lru_list_t *lru);

// 获取最近使用的元素（链表头部元素）
extern struct lru_node* lru_get_mru(const lru_list_t *lru);

// 遍历LRU链表（从最近使用到最少使用）
#define lru_for_each(pos, lru, type, member) \
    list_for_each_entry(pos, &(lru)->list, member.list)

// 安全遍历LRU链表（可在遍历过程中删除节点）
#define lru_for_each_safe(pos, n, lru, type, member) \
    list_for_each_entry_safe(pos, n, &(lru)->list, member.list)

#endif // __LRU_LIST_H__