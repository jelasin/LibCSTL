#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include "rb_tree.h"
#include <stdlib.h>

/**
 * 哈希映射状态码
 */
typedef enum {
    HASHMAP_OK = 0,            // 操作成功
    HASHMAP_ERR = -1,          // 一般错误
    HASHMAP_DUPLICATE = -2,    // 键已存在
    HASHMAP_NOT_FOUND = -3,    // 键不存在
    HASHMAP_NOMEM = -4,        // 内存分配失败
} hashmap_status_t;

/**
 * 哈希表节点结构
 */
typedef struct hashmap_entry {
    struct rb_node rb_node;   // 红黑树节点
    unsigned int hash;        // 键的哈希值
    void *key;                // 键
    void *value;              // 值
    size_t key_size;          // 键大小
    size_t value_size;        // 值大小
} hashmap_entry_t;

/**
 * 哈希函数类型定义
 */
typedef unsigned int (*hashmap_hash_fn)(const void *key, size_t key_size);

/**
 * 键比较函数类型定义
 */
typedef int (*hashmap_key_compare_fn)(const void *key1, size_t key1_size, 
                                     const void *key2, size_t key2_size);

/**
 * 哈希映射结构
 */
typedef struct hashmap {
    size_t size;              // 键值对数量
    size_t capacity;          // 桶数组容量
    float load_factor;        // 负载因子
    rb_root_t *buckets;       // 桶数组（红黑树）
    
    hashmap_hash_fn hash_fn;  // 哈希函数
    hashmap_key_compare_fn key_compare_fn; // 键比较函数
    
    // 内存操作函数
    void* (*key_dup)(const void *key, size_t size);
    void* (*value_dup)(const void *value, size_t size);
    void (*key_free)(void *key);
    void (*value_free)(void *value);
} hashmap_t;

/**
 * 遍历回调函数类型
 */
typedef bool (*hashmap_foreach_fn)(const void *key, size_t key_size,
                                  void *value, size_t value_size,
                                  void *user_data);

/**
 * 创建哈希映射
 * @param initial_capacity 初始桶数量
 * @param load_factor 负载因子，超过此值将重新哈希
 * @param hash_fn 哈希函数
 * @param key_compare_fn 键比较函数
 * @return 哈希映射对象，失败返回NULL
 */
hashmap_t* hashmap_create(size_t initial_capacity, float load_factor,
                         hashmap_hash_fn hash_fn,
                         hashmap_key_compare_fn key_compare_fn);

/**
 * 销毁哈希映射
 * @param map 哈希映射对象
 */
void hashmap_destroy(hashmap_t *map);

/**
 * 清空哈希映射
 * @param map 哈希映射对象
 */
void hashmap_clear(hashmap_t *map);

/**
 * 获取哈希映射中键值对数量
 * @param map 哈希映射对象
 * @return 键值对数量
 */
size_t hashmap_size(const hashmap_t *map);

/**
 * 插入键值对
 * @param map 哈希映射对象
 * @param key 键
 * @param key_size 键的大小
 * @param value 值
 * @param value_size 值的大小
 * @return 状态码
 */
hashmap_status_t hashmap_put(hashmap_t *map, const void *key, size_t key_size,
                            const void *value, size_t value_size);

/**
 * 获取键对应的值
 * @param map 哈希映射对象
 * @param key 键
 * @param key_size 键的大小
 * @param value 用于存储值的缓冲区
 * @param value_size 缓冲区大小
 * @param actual_size 实际值的大小（可为NULL）
 * @return 状态码
 */
hashmap_status_t hashmap_get(const hashmap_t *map, const void *key, size_t key_size,
                           void *value, size_t value_size, size_t *actual_size);

/**
 * 移除键值对
 * @param map 哈希映射对象
 * @param key 键
 * @param key_size 键的大小
 * @return 状态码
 */
hashmap_status_t hashmap_remove(hashmap_t *map, const void *key, size_t key_size);

/**
 * 检查键是否存在
 * @param map 哈希映射对象
 * @param key 键
 * @param key_size 键的大小
 * @return true表示存在，false表示不存在
 */
bool hashmap_contains(const hashmap_t *map, const void *key, size_t key_size);

/**
 * 遍历哈希映射
 * @param map 哈希映射对象
 * @param fn 回调函数
 * @param user_data 用户数据
 */
void hashmap_foreach(const hashmap_t *map, hashmap_foreach_fn fn, void *user_data);

/**
 * 重新设置哈希映射容量
 * @param map 哈希映射对象
 * @param new_capacity 新容量
 * @return 状态码
 */
hashmap_status_t hashmap_resize(hashmap_t *map, size_t new_capacity);

/**
 * 设置自定义内存操作函数
 * @param map 哈希映射对象
 * @param key_dup 键复制函数
 * @param value_dup 值复制函数
 * @param key_free 键释放函数
 * @param value_free 值释放函数
 */
void hashmap_set_memory_functions(hashmap_t *map,
                                 void* (*key_dup)(const void *key, size_t size),
                                 void* (*value_dup)(const void *value, size_t size),
                                 void (*key_free)(void *key),
                                 void (*value_free)(void *value));

/**
 * 默认字符串哈希函数
 * @param key 键
 * @param key_size 键的大小
 * @return 哈希值
 */
unsigned int hashmap_hash_string(const void *key, size_t key_size);

/**
 * 默认数据哈希函数
 * @param key 键
 * @param key_size 键的大小
 * @return 哈希值
 */
unsigned int hashmap_hash_data(const void *key, size_t key_size);

/**
 * 默认字符串比较函数
 * @param key1 第一个键
 * @param key1_size 第一个键的大小
 * @param key2 第二个键
 * @param key2_size 第二个键的大小
 * @return 比较结果
 */
int hashmap_compare_string(const void *key1, size_t key1_size, 
                          const void *key2, size_t key2_size);

/**
 * 默认数据比较函数
 * @param key1 第一个键
 * @param key1_size 第一个键的大小
 * @param key2 第二个键
 * @param key2_size 第二个键的大小
 * @return 比较结果
 */
int hashmap_compare_data(const void *key1, size_t key1_size, 
                        const void *key2, size_t key2_size);

#endif /* __HASHMAP_H__ */