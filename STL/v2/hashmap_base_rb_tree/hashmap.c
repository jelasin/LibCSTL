#include "hashmap.h"
#include <string.h>
#include <stdio.h>

#define HASHMAP_DEFAULT_CAPACITY 16
#define HASHMAP_DEFAULT_LOAD_FACTOR 0.75f
#define HASHMAP_RESIZE_FACTOR 2
#define HASHMAP_MIN_CAPACITY 8

// 红黑树节点比较函数
static int rb_node_compare(const struct rb_node *a, const struct rb_node *b, void *arg) {
    hashmap_t *map = (hashmap_t*)arg;
    hashmap_entry_t *entry_a = rb_entry(a, hashmap_entry_t, rb_node);
    hashmap_entry_t *entry_b = rb_entry(b, hashmap_entry_t, rb_node);
    
    // 先比较哈希值
    if (entry_a->hash != entry_b->hash) {
        return entry_a->hash - entry_b->hash;
    }
    
    // 哈希值相同，再比较键内容
    return map->key_compare_fn(entry_a->key, entry_a->key_size, 
                              entry_b->key, entry_b->key_size);
}

// 红黑树节点析构函数
static void rb_node_destructor(struct rb_node *node, void *arg) {
    hashmap_t *map = (hashmap_t*)arg;
    hashmap_entry_t *entry = rb_entry(node, hashmap_entry_t, rb_node);
    
    // 释放键和值
    if (map->key_free) {
        map->key_free(entry->key);
    }
    if (map->value_free) {
        map->value_free(entry->value);
    }
    
    // 释放节点
    free(entry);
}

// 默认键复制函数
static void* default_key_dup(const void *key, size_t size) {
    void *new_key = malloc(size);
    if (new_key) {
        memcpy(new_key, key, size);
    }
    return new_key;
}

// 默认值复制函数
static void* default_value_dup(const void *value, size_t size) {
    void *new_value = malloc(size);
    if (new_value) {
        memcpy(new_value, value, size);
    }
    return new_value;
}

// 默认键释放函数
static void default_key_free(void *key) {
    free(key);
}

// 默认值释放函数
static void default_value_free(void *value) {
    free(value);
}

// 字符串哈希函数 - DJB2算法
unsigned int hashmap_hash_string(const void *key, size_t key_size) {
    const unsigned char *str = (const unsigned char *)key;
    unsigned int hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    return hash;
}

// 数据哈希函数
unsigned int hashmap_hash_data(const void *key, size_t key_size) {
    const unsigned char *data = (const unsigned char *)key;
    unsigned int hash = 5381;
    
    for (size_t i = 0; i < key_size; i++) {
        hash = ((hash << 5) + hash) + data[i];
    }
    
    return hash;
}

// 字符串比较函数
int hashmap_compare_string(const void *key1, size_t key1_size, 
                          const void *key2, size_t key2_size) {
    return strcmp((const char *)key1, (const char *)key2);
}

// 数据比较函数
int hashmap_compare_data(const void *key1, size_t key1_size, 
                        const void *key2, size_t key2_size) {
    if (key1_size != key2_size) {
        return key1_size - key2_size;
    }
    return memcmp(key1, key2, key1_size);
}

// 创建哈希映射
hashmap_t* hashmap_create(size_t initial_capacity, float load_factor,
                         hashmap_hash_fn hash_fn,
                         hashmap_key_compare_fn key_compare_fn) {
    // 参数验证
    if (initial_capacity < HASHMAP_MIN_CAPACITY) {
        initial_capacity = HASHMAP_MIN_CAPACITY;
    }
    if (load_factor <= 0.0f || load_factor > 1.0f) {
        load_factor = HASHMAP_DEFAULT_LOAD_FACTOR;
    }
    
    // 分配映射结构
    hashmap_t *map = (hashmap_t*)malloc(sizeof(hashmap_t));
    if (!map) {
        return NULL;
    }
    
    // 分配桶数组
    map->buckets = (rb_root_t*)calloc(initial_capacity, sizeof(rb_root_t));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    
    // 初始化属性
    map->size = 0;
    map->capacity = initial_capacity;
    map->load_factor = load_factor;
    map->hash_fn = hash_fn ? hash_fn : hashmap_hash_string;
    map->key_compare_fn = key_compare_fn ? key_compare_fn : hashmap_compare_string;
    
    // 设置默认的内存操作函数
    map->key_dup = default_key_dup;
    map->value_dup = default_value_dup;
    map->key_free = default_key_free;
    map->value_free = default_value_free;
    
    // 初始化桶
    for (size_t i = 0; i < initial_capacity; i++) {
        rb_init(&map->buckets[i], rb_node_compare, map, rb_node_destructor, map);
    }
    
    return map;
}

// 销毁哈希映射
void hashmap_destroy(hashmap_t *map) {
    if (!map) {
        return;
    }
    
    // 清除所有桶
    for (size_t i = 0; i < map->capacity; i++) {
        rb_destroy(&map->buckets[i]);
    }
    
    // 释放桶数组和映射结构
    free(map->buckets);
    free(map);
}

// 清空哈希映射
void hashmap_clear(hashmap_t *map) {
    if (!map) {
        return;
    }
    
    // 清除所有桶
    for (size_t i = 0; i < map->capacity; i++) {
        rb_destroy(&map->buckets[i]);
        rb_init(&map->buckets[i], rb_node_compare, map, rb_node_destructor, map);
    }
    
    map->size = 0;
}

// 获取哈希映射中键值对数量
size_t hashmap_size(const hashmap_t *map) {
    return map ? map->size : 0;
}

// 查找键对应的节点
static hashmap_entry_t* hashmap_find_entry(const hashmap_t *map, 
                                         const void *key, size_t key_size,
                                         unsigned int *hash_out) {
    // 计算哈希值
    unsigned int hash = map->hash_fn(key, key_size);
    if (hash_out) {
        *hash_out = hash;
    }
    
    // 确定桶索引
    size_t index = hash % map->capacity;
    
    // 创建一个临时节点用于查找
    hashmap_entry_t temp = {
        .hash = hash,
        .key = (void*)key,
        .key_size = key_size
    };
    
    // 在红黑树中查找
    struct rb_node *node = rb_search(&map->buckets[index], &temp.rb_node);
    if (!node) {
        return NULL;
    }
    
    return rb_entry(node, hashmap_entry_t, rb_node);
}

// 执行重新哈希
static hashmap_status_t hashmap_rehash(hashmap_t *map, size_t new_capacity) {
    // 参数验证
    if (new_capacity < HASHMAP_MIN_CAPACITY) {
        new_capacity = HASHMAP_MIN_CAPACITY;
    }
    
    // 分配新桶
    rb_root_t *new_buckets = (rb_root_t*)calloc(new_capacity, sizeof(rb_root_t));
    if (!new_buckets) {
        return HASHMAP_NOMEM;
    }
    
    // 初始化新桶
    for (size_t i = 0; i < new_capacity; i++) {
        rb_init(&new_buckets[i], rb_node_compare, map, rb_node_destructor, map);
    }
    
    // 遍历旧桶，重新哈希所有项
    for (size_t i = 0; i < map->capacity; i++) {
        struct rb_node *node = rb_first(&map->buckets[i]);
        while (node) {
            // 保存下一个节点
            struct rb_node *next = rb_next(node);
            
            // 重新计算桶索引
            hashmap_entry_t *entry = rb_entry(node, hashmap_entry_t, rb_node);
            size_t new_index = entry->hash % new_capacity;
            
            // 从旧桶中删除
            rb_erase(&map->buckets[i], node);
            
            // 插入到新桶
            rb_insert(&new_buckets[new_index], node);
            
            // 继续下一个节点
            node = next;
        }
    }
    
    // 释放旧桶数组
    free(map->buckets);
    
    // 更新映射属性
    map->buckets = new_buckets;
    map->capacity = new_capacity;
    
    return HASHMAP_OK;
}

// 插入键值对
hashmap_status_t hashmap_put(hashmap_t *map, const void *key, size_t key_size,
                            const void *value, size_t value_size) {
    if (!map || !key) {
        return HASHMAP_ERR;
    }
    
    // 检查是否需要扩容
    if ((float)(map->size + 1) / map->capacity > map->load_factor) {
        hashmap_status_t status = hashmap_rehash(map, map->capacity * HASHMAP_RESIZE_FACTOR);
        if (status != HASHMAP_OK) {
            return status;
        }
    }
    
    // 计算哈希值
    unsigned int hash;
    hashmap_entry_t *existing = hashmap_find_entry(map, key, key_size, &hash);
    
    if (existing) {
        // 更新现有值
        void *new_value = map->value_dup(value, value_size);
        if (!new_value) {
            return HASHMAP_NOMEM;
        }
        
        // 释放旧值
        if (map->value_free) {
            map->value_free(existing->value);
        }
        
        existing->value = new_value;
        existing->value_size = value_size;
        
        return HASHMAP_OK;
    } else {
        // 创建新节点
        hashmap_entry_t *entry = (hashmap_entry_t*)malloc(sizeof(hashmap_entry_t));
        if (!entry) {
            return HASHMAP_NOMEM;
        }
        
        // 复制键和值
        entry->key = map->key_dup(key, key_size);
        if (!entry->key) {
            free(entry);
            return HASHMAP_NOMEM;
        }
        
        entry->value = map->value_dup(value, value_size);
        if (!entry->value) {
            map->key_free(entry->key);
            free(entry);
            return HASHMAP_NOMEM;
        }
        
        // 设置节点属性
        entry->hash = hash;
        entry->key_size = key_size;
        entry->value_size = value_size;
        
        // 确定桶索引
        size_t index = hash % map->capacity;
        
        // 插入红黑树
        if (rb_insert(&map->buckets[index], &entry->rb_node) != 0) {
            map->key_free(entry->key);
            map->value_free(entry->value);
            free(entry);
            return HASHMAP_ERR;
        }
        
        map->size++;
        return HASHMAP_OK;
    }
}

// 获取键对应的值
hashmap_status_t hashmap_get(const hashmap_t *map, const void *key, size_t key_size,
                           void *value, size_t value_size, size_t *actual_size) {
    if (!map || !key) {
        return HASHMAP_ERR;
    }
    
    hashmap_entry_t *entry = hashmap_find_entry(map, key, key_size, NULL);
    if (!entry) {
        return HASHMAP_NOT_FOUND;
    }
    
    // 返回实际值大小
    if (actual_size) {
        *actual_size = entry->value_size;
    }
    
    // 复制值
    if (value) {
        size_t copy_size = (value_size < entry->value_size) ? value_size : entry->value_size;
        memcpy(value, entry->value, copy_size);
    }
    
    return HASHMAP_OK;
}

// 移除键值对
hashmap_status_t hashmap_remove(hashmap_t *map, const void *key, size_t key_size) {
    if (!map || !key) {
        return HASHMAP_ERR;
    }
    
    // 计算哈希值
    unsigned int hash = map->hash_fn(key, key_size);
    size_t index = hash % map->capacity;
    
    // 创建临时节点用于查找
    hashmap_entry_t temp = {
        .hash = hash,
        .key = (void*)key,
        .key_size = key_size
    };
    
    // 在红黑树中查找
    struct rb_node *node = rb_search(&map->buckets[index], &temp.rb_node);
    if (!node) {
        return HASHMAP_NOT_FOUND;
    }
    
    // 从红黑树中删除
    rb_erase(&map->buckets[index], node);
    map->size--;
    
    return HASHMAP_OK;
}

// 检查键是否存在
bool hashmap_contains(const hashmap_t *map, const void *key, size_t key_size) {
    if (!map || !key) {
        return false;
    }
    
    return hashmap_find_entry(map, key, key_size, NULL) != NULL;
}

// 遍历哈希映射
void hashmap_foreach(const hashmap_t *map, hashmap_foreach_fn fn, void *user_data) {
    if (!map || !fn) {
        return;
    }
    
    // 遍历所有桶
    for (size_t i = 0; i < map->capacity; i++) {
        struct rb_node *node = rb_first(&map->buckets[i]);
        while (node) {
            hashmap_entry_t *entry = rb_entry(node, hashmap_entry_t, rb_node);
            node = rb_next(node); // 先保存下一个节点，因为回调可能导致当前节点被删除
            
            // 调用回调函数
            if (!fn(entry->key, entry->key_size, entry->value, entry->value_size, user_data)) {
                return; // 中断遍历
            }
        }
    }
}

// 重新设置哈希映射容量
hashmap_status_t hashmap_resize(hashmap_t *map, size_t new_capacity) {
    if (!map) {
        return HASHMAP_ERR;
    }
    
    return hashmap_rehash(map, new_capacity);
}

// 设置自定义内存操作函数
void hashmap_set_memory_functions(hashmap_t *map,
                                 void* (*key_dup)(const void *key, size_t size),
                                 void* (*value_dup)(const void *value, size_t size),
                                 void (*key_free)(void *key),
                                 void (*value_free)(void *value)) {
    if (!map) {
        return;
    }
    
    map->key_dup = key_dup ? key_dup : default_key_dup;
    map->value_dup = value_dup ? value_dup : default_value_dup;
    map->key_free = key_free ? key_free : default_key_free;
    map->value_free = value_free ? value_free : default_value_free;
}