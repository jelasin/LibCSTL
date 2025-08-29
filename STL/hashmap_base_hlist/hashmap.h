#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdlib.h>
#include <stdbool.h>

/**
 * 哈希表结构
 */
typedef struct hashmap hashmap_t;

/**
 * 哈希函数类型
 * @param key 键
 * @param key_len 键的长度
 * @param userdata 用户数据
 * @return 哈希值
 */
typedef unsigned int (*hashmap_hash_fn)(const void *key, size_t key_len, void *userdata);

/**
 * 键比较函数类型
 * @param key1 第一个键
 * @param key_len1 第一个键的长度
 * @param key2 第二个键
 * @param key_len2 第二个键的长度
 * @param userdata 用户数据
 * @return 0表示相等，非0表示不相等
 */
typedef int (*hashmap_key_compare_fn)(const void *key1, size_t key_len1, 
                                    const void *key2, size_t key_len2,
                                    void *userdata);

/**
 * 键复制函数类型
 * @param key 原键
 * @param key_len 键的长度
 * @param userdata 用户数据
 * @return 复制的键
 */
typedef void* (*hashmap_key_dup_fn)(const void *key, size_t key_len, void *userdata);

/**
 * 值复制函数类型
 * @param value 原值
 * @param value_len 值的长度
 * @param userdata 用户数据
 * @return 复制的值
 */
typedef void* (*hashmap_value_dup_fn)(const void *value, size_t value_len, void *userdata);

/**
 * 键释放函数类型
 * @param key 要释放的键
 * @param userdata 用户数据
 */
typedef void (*hashmap_key_free_fn)(void *key, void *userdata);

/**
 * 值释放函数类型
 * @param value 要释放的值
 * @param userdata 用户数据
 */
typedef void (*hashmap_value_free_fn)(void *value, void *userdata);

/**
 * 遍历回调函数类型
 * @param key 键
 * @param key_len 键的长度
 * @param value 值
 * @param value_len 值的长度
 * @param userdata 用户数据
 * @return 如果返回true继续遍历，返回false停止遍历
 */
typedef bool (*hashmap_foreach_fn)(const void *key, size_t key_len, 
                                 void *value, size_t value_len,
                                 void *userdata);

/**
 * 哈希表选项
 */
typedef struct {
    unsigned int initial_size;          // 初始大小
    float load_factor;                  // 负载因子，超过此值将扩展表
    hashmap_hash_fn hash_fn;           // 哈希函数
    hashmap_key_compare_fn key_compare; // 键比较函数
    hashmap_key_dup_fn key_dup;        // 键复制函数
    hashmap_value_dup_fn value_dup;    // 值复制函数
    hashmap_key_free_fn key_free;      // 键释放函数
    hashmap_value_free_fn value_free;  // 值释放函数
    void *userdata;                    // 用户数据，传递给回调函数
} hashmap_options_t;

/**
 * 创建哈希表
 * @param options 选项，如果为NULL则使用默认选项
 * @return 哈希表句柄，失败返回NULL
 */
hashmap_t* hashmap_create(const hashmap_options_t *options);

/**
 * 销毁哈希表
 * @param map 哈希表句柄
 */
void hashmap_destroy(hashmap_t *map);

/**
 * 清空哈希表
 * @param map 哈希表句柄
 */
void hashmap_clear(hashmap_t *map);

/**
 * 插入键值对
 * @param map 哈希表句柄
 * @param key 键
 * @param key_len 键的长度
 * @param value 值
 * @param value_len 值的长度
 * @return 成功返回0，失败返回非0值
 */
int hashmap_put(hashmap_t *map, const void *key, size_t key_len, 
                const void *value, size_t value_len);

/**
 * 删除键值对
 * @param map 哈希表句柄
 * @param key 键
 * @param key_len 键的长度
 * @return 成功返回0，未找到返回-1
 */
int hashmap_remove(hashmap_t *map, const void *key, size_t key_len);

/**
 * 获取值
 * @param map 哈希表句柄
 * @param key 键
 * @param key_len 键的长度
 * @param value 用于存储值的缓冲区
 * @param value_size 缓冲区大小
 * @param value_len 如果非NULL，用于返回实际值长度
 * @return 成功返回0，未找到返回-1，缓冲区太小返回-2
 */
int hashmap_get(hashmap_t *map, const void *key, size_t key_len, 
                void *value, size_t value_size, size_t *value_len);

/**
 * 检查键是否存在
 * @param map 哈希表句柄
 * @param key 键
 * @param key_len 键的长度
 * @return 存在返回true，不存在返回false
 */
bool hashmap_contains(hashmap_t *map, const void *key, size_t key_len);

/**
 * 遍历哈希表
 * @param map 哈希表句柄
 * @param callback 回调函数
 * @param userdata 用户数据
 */
void hashmap_foreach(hashmap_t *map, hashmap_foreach_fn callback, void *userdata);

/**
 * 获取哈希表大小
 * @param map 哈希表句柄
 * @return 键值对数量
 */
size_t hashmap_size(const hashmap_t *map);

/**
 * 获取当前哈希表容量
 * @param map 哈希表句柄
 * @return 当前桶数量
 */
size_t hashmap_capacity(const hashmap_t *map);

/**
 * 设置哈希表容量
 * @param map 哈希表句柄
 * @param capacity 新容量
 * @return 成功返回0，失败返回非0值
 */
int hashmap_resize(hashmap_t *map, size_t capacity);

/**
 * 字符串哈希函数（默认）
 */
unsigned int hashmap_hash_string(const void *key, size_t key_len, void *userdata);

/**
 * 通用内存哈希函数
 */
unsigned int hashmap_hash_data(const void *key, size_t key_len, void *userdata);

/**
 * 字符串比较函数（默认）
 */
int hashmap_compare_string(const void *key1, size_t key_len1, 
                          const void *key2, size_t key_len2, 
                          void *userdata);

/**
 * 通用内存比较函数
 */
int hashmap_compare_data(const void *key1, size_t key_len1, 
                        const void *key2, size_t key_len2, 
                        void *userdata);

#endif // __HASHMAP_H__