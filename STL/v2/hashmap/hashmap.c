#include "hashmap.h"
#include "hlist.h"
#include <string.h>
#include <stdio.h>

#define HASHMAP_DEFAULT_INITIAL_SIZE 16
#define HASHMAP_DEFAULT_LOAD_FACTOR 0.75f
#define HASHMAP_MIN_CAPACITY 8
#define HASHMAP_GROWTH_FACTOR 2

/**
 * 哈希表节点结构
 */
typedef struct hashmap_entry {
    hlist_node node;     // 哈希链表节点
    void *key;           // 键
    size_t key_len;      // 键长度
    void *value;         // 值
    size_t value_len;    // 值长度
} hashmap_entry_t;

/**
 * 哈希表结构
 */
struct hashmap {
    hlist_head *buckets;        // 桶数组
    size_t bucket_count;        // 桶数量
    size_t size;                // 键值对数量
    float load_factor;          // 负载因子
    
    hashmap_hash_fn hash_fn;           // 哈希函数
    hashmap_key_compare_fn key_compare; // 键比较函数
    hashmap_key_dup_fn key_dup;        // 键复制函数
    hashmap_value_dup_fn value_dup;    // 值复制函数
    hashmap_key_free_fn key_free;      // 键释放函数
    hashmap_value_free_fn value_free;  // 值释放函数
    void *userdata;                    // 用户数据
};

// 默认字符串哈希函数
unsigned int hashmap_hash_string(const void *key, size_t key_len, void *userdata) {
    const char *str = (const char *)key;
    unsigned int hash = 5381;
    int c;
    
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    
    return hash;
}

// 默认数据哈希函数
unsigned int hashmap_hash_data(const void *key, size_t key_len, void *userdata) {
    const unsigned char *data = (const unsigned char *)key;
    unsigned int hash = 5381;
    
    for (size_t i = 0; i < key_len; i++)
        hash = ((hash << 5) + hash) + data[i];
    
    return hash;
}

// 默认字符串比较函数
int hashmap_compare_string(const void *key1, size_t key_len1, 
                          const void *key2, size_t key_len2, 
                          void *userdata) {
    return strcmp((const char *)key1, (const char *)key2);
}

// 默认数据比较函数
int hashmap_compare_data(const void *key1, size_t key_len1, 
                        const void *key2, size_t key_len2, 
                        void *userdata) {
    if (key_len1 != key_len2)
        return key_len1 - key_len2;
    return memcmp(key1, key2, key_len1);
}

// 默认键复制函数
static void* hashmap_default_key_dup(const void *key, size_t key_len, void *userdata) {
    void *new_key = malloc(key_len);
    if (new_key)
        memcpy(new_key, key, key_len);
    return new_key;
}

// 默认值复制函数
static void* hashmap_default_value_dup(const void *value, size_t value_len, void *userdata) {
    void *new_value = malloc(value_len);
    if (new_value)
        memcpy(new_value, value, value_len);
    return new_value;
}

// 默认键释放函数
static void hashmap_default_key_free(void *key, void *userdata) {
    free(key);
}

// 默认值释放函数
static void hashmap_default_value_free(void *value, void *userdata) {
    free(value);
}

// 创建哈希表
hashmap_t* hashmap_create(const hashmap_options_t *options) {
    hashmap_t *map = (hashmap_t*)malloc(sizeof(hashmap_t));
    if (!map)
        return NULL;
    
    // 使用默认选项或用户提供的选项
    unsigned int initial_size = options && options->initial_size > 0 ? 
                                options->initial_size : HASHMAP_DEFAULT_INITIAL_SIZE;
    
    if (initial_size < HASHMAP_MIN_CAPACITY)
        initial_size = HASHMAP_MIN_CAPACITY;
    
    map->bucket_count = initial_size;
    map->size = 0;
    map->load_factor = options && options->load_factor > 0 ? 
                       options->load_factor : HASHMAP_DEFAULT_LOAD_FACTOR;
    
    map->hash_fn = options && options->hash_fn ? 
                   options->hash_fn : hashmap_hash_string;
    
    map->key_compare = options && options->key_compare ? 
                       options->key_compare : hashmap_compare_string;
    
    map->key_dup = options && options->key_dup ? 
                   options->key_dup : hashmap_default_key_dup;
    
    map->value_dup = options && options->value_dup ? 
                     options->value_dup : hashmap_default_value_dup;
    
    map->key_free = options && options->key_free ? 
                    options->key_free : hashmap_default_key_free;
    
    map->value_free = options && options->value_free ? 
                      options->value_free : hashmap_default_value_free;
    
    map->userdata = options ? options->userdata : NULL;
    
    // 分配桶数组
    map->buckets = (hlist_head*)malloc(map->bucket_count * sizeof(hlist_head));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    
    // 初始化桶
    for (size_t i = 0; i < map->bucket_count; i++) {
        hlist_init_head(&map->buckets[i]);
    }
    
    return map;
}

// 查找入口点
static hashmap_entry_t* hashmap_find_entry(hashmap_t *map, const void *key, size_t key_len, unsigned int *hash_out) {
    if (!map || !key)
        return NULL;
    
    unsigned int hash = map->hash_fn(key, key_len, map->userdata);
    if (hash_out)
        *hash_out = hash;
    
    unsigned int index = hash % map->bucket_count;
    hashmap_entry_t *entry;
    
    hlist_head *head = &map->buckets[index];
    hlist_node *pos;
    
    hlist_for_each(pos, head) {
        entry = hlist_entry(pos, hashmap_entry_t, node);
        if (map->key_compare(entry->key, entry->key_len, key, key_len, map->userdata) == 0) {
            return entry;
        }
    }
    
    return NULL;
}

// 扩展哈希表
static int hashmap_resize_internal(hashmap_t *map, size_t new_size) {
    if (!map || new_size < HASHMAP_MIN_CAPACITY)
        return -1;
    
    // 创建新桶数组
    hlist_head *new_buckets = (hlist_head*)malloc(new_size * sizeof(hlist_head));
    if (!new_buckets)
        return -1;
    
    // 初始化新桶
    for (size_t i = 0; i < new_size; i++) {
        hlist_init_head(&new_buckets[i]);
    }
    
    // 遍历所有旧桶，重新哈希
    for (size_t i = 0; i < map->bucket_count; i++) {
        hlist_head *head = &map->buckets[i];
        hlist_node *pos, *tmp;
        
        hlist_for_each_safe(pos, tmp, head) {
            hashmap_entry_t *entry = hlist_entry(pos, hashmap_entry_t, node);
            
            // 从旧桶中删除
            hlist_del(pos);
            
            // 计算在新桶中的位置
            unsigned int hash = map->hash_fn(entry->key, entry->key_len, map->userdata);
            unsigned int new_index = hash % new_size;
            
            // 添加到新桶
            hlist_add_head(pos, &new_buckets[new_index]);
        }
    }
    
    // 释放旧桶数组并更新映射
    free(map->buckets);
    map->buckets = new_buckets;
    map->bucket_count = new_size;
    
    return 0;
}

// 销毁哈希表
void hashmap_destroy(hashmap_t *map) {
    if (!map)
        return;
    
    // 清除所有条目
    hashmap_clear(map);
    
    // 释放桶数组
    free(map->buckets);
    
    // 释放映射结构
    free(map);
}

// 清空哈希表
void hashmap_clear(hashmap_t *map) {
    if (!map)
        return;
    
    // 遍历所有桶，释放条目
    for (size_t i = 0; i < map->bucket_count; i++) {
        hlist_head *head = &map->buckets[i];
        hlist_node *pos, *tmp;
        
        hlist_for_each_safe(pos, tmp, head) {
            hashmap_entry_t *entry = hlist_entry(pos, hashmap_entry_t, node);
            
            // 从桶中删除
            hlist_del(pos);
            
            // 释放键和值
            if (map->key_free)
                map->key_free(entry->key, map->userdata);
            
            if (map->value_free)
                map->value_free(entry->value, map->userdata);
            
            // 释放条目
            free(entry);
        }
    }
    
    map->size = 0;
}

// 插入键值对
int hashmap_put(hashmap_t *map, const void *key, size_t key_len, 
                const void *value, size_t value_len) {
    if (!map || !key)
        return -1;
    
    // 检查是否需要扩展表
    if ((float)(map->size + 1) / map->bucket_count > map->load_factor) {
        hashmap_resize_internal(map, map->bucket_count * HASHMAP_GROWTH_FACTOR);
    }
    
    unsigned int hash;
    hashmap_entry_t *existing = hashmap_find_entry(map, key, key_len, &hash);
    
    if (existing) {
        // 更新现有条目
        void *new_value = map->value_dup(value, value_len, map->userdata);
        if (!new_value)
            return -1;
        
        if (map->value_free)
            map->value_free(existing->value, map->userdata);
        
        existing->value = new_value;
        existing->value_len = value_len;
    } else {
        // 创建新条目
        hashmap_entry_t *entry = (hashmap_entry_t*)malloc(sizeof(hashmap_entry_t));
        if (!entry)
            return -1;
        
        // 复制键和值
        entry->key = map->key_dup(key, key_len, map->userdata);
        if (!entry->key) {
            free(entry);
            return -1;
        }
        
        entry->value = map->value_dup(value, value_len, map->userdata);
        if (!entry->value) {
            map->key_free(entry->key, map->userdata);
            free(entry);
            return -1;
        }
        
        entry->key_len = key_len;
        entry->value_len = value_len;
        
        // 添加到哈希表
        unsigned int index = hash % map->bucket_count;
        hlist_add_head(&entry->node, &map->buckets[index]);
        
        map->size++;
    }
    
    return 0;
}

// 删除键值对
int hashmap_remove(hashmap_t *map, const void *key, size_t key_len) {
    if (!map || !key)
        return -1;
    
    hashmap_entry_t *entry = hashmap_find_entry(map, key, key_len, NULL);
    if (!entry)
        return -1;  // 键不存在
    
    // 从哈希表中删除
    hlist_del(&entry->node);
    
    // 释放键和值
    if (map->key_free)
        map->key_free(entry->key, map->userdata);
    
    if (map->value_free)
        map->value_free(entry->value, map->userdata);
    
    // 释放条目
    free(entry);
    
    map->size--;
    
    return 0;
}

// 获取值
int hashmap_get(hashmap_t *map, const void *key, size_t key_len, 
                void *value, size_t value_size, size_t *value_len) {
    if (!map || !key)
        return -1;
    
    hashmap_entry_t *entry = hashmap_find_entry(map, key, key_len, NULL);
    if (!entry)
        return -1;  // 键不存在
    
    // 如果提供了长度指针，返回实际值长度
    if (value_len)
        *value_len = entry->value_len;
    
    // 如果需要返回值
    if (value) {
        if (value_size < entry->value_len)
            return -2;  // 缓冲区太小
        
        memcpy(value, entry->value, entry->value_len);
    }
    
    return 0;
}

// 检查键是否存在
bool hashmap_contains(hashmap_t *map, const void *key, size_t key_len) {
    if (!map || !key)
        return false;
    
    return hashmap_find_entry(map, key, key_len, NULL) != NULL;
}

// 遍历哈希表
void hashmap_foreach(hashmap_t *map, hashmap_foreach_fn callback, void *userdata) {
    if (!map || !callback)
        return;
    
    for (size_t i = 0; i < map->bucket_count; i++) {
        hlist_head *head = &map->buckets[i];
        hlist_node *pos;
        
        hlist_for_each(pos, head) {
            hashmap_entry_t *entry = hlist_entry(pos, hashmap_entry_t, node);
            
            if (!callback(entry->key, entry->key_len, entry->value, entry->value_len, userdata))
                return;  // 回调要求停止遍历
        }
    }
}

// 获取哈希表大小
size_t hashmap_size(const hashmap_t *map) {
    return map ? map->size : 0;
}

// 获取当前哈希表容量
size_t hashmap_capacity(const hashmap_t *map) {
    return map ? map->bucket_count : 0;
}

// 设置哈希表容量
int hashmap_resize(hashmap_t *map, size_t capacity) {
    if (!map)
        return -1;
    
    return hashmap_resize_internal(map, capacity);
}