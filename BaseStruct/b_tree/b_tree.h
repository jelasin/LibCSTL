#ifndef __B_TREE_H__
#define __B_TREE_H__

#include <stdlib.h>
#include <stdbool.h>

#ifndef offsetof
typedef unsigned long size_t;
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({          \
        const typeof( ((type *)0)->member ) *__mptr = (const typeof( ((type *)0)->member ) *)(ptr); \
        (type *)( (char *)__mptr - offsetof(type, member) );})
#endif

#ifndef btree_entry
#define btree_entry(ptr, type, member) \
    container_of(ptr, type, member)
#endif

// B树的阶（最大子节点数）
#define BTREE_DEFAULT_ORDER 5

// B树节点结构
struct btree_node {
    int n;                          // 当前关键字数量
    bool is_leaf;                   // 是否是叶节点
    void **keys;                    // 关键字数组
    struct btree_node **children;   // 子节点指针数组
};

// B树结构
typedef struct btree {
    struct btree_node *root;        // 根节点
    int t;                          // 最小度数 (order = 2*t-1)
    int order;                      // B树的阶
    
    // 比较函数，返回值：
    // < 0: a < b
    // = 0: a == b
    // > 0: a > b
    int (*compare)(const void *a, const void *b, void *arg);
    void *compare_arg;              // 比较函数参数
    
    // 析构函数，用于释放键的内存（如果需要）
    void (*key_destructor)(void *key, void *arg);
    void *destructor_arg;           // 析构函数参数
} btree_t;

// 创建B树
extern btree_t* btree_create(int order, 
                            int (*compare)(const void *a, const void *b, void *arg),
                            void *compare_arg,
                            void (*key_destructor)(void *key, void *arg),
                            void *destructor_arg);

// 销毁B树
extern void btree_destroy(btree_t *tree);

// 插入关键字
extern int btree_insert(btree_t *tree, void *key);

// 删除关键字
extern int btree_delete(btree_t *tree, const void *key);

// 搜索关键字，返回找到的键
extern void* btree_search(const btree_t *tree, const void *key);

// 获取B树中的最小关键字
extern void* btree_get_min(const btree_t *tree);

// 获取B树中的最大关键字
extern void* btree_get_max(const btree_t *tree);

// 获取B树的高度
extern int btree_height(const btree_t *tree);

// 获取B树中关键字的数量
extern int btree_count(const btree_t *tree);

// 清空B树
extern void btree_clear(btree_t *tree);

// 检查B树是否为空
extern bool btree_empty(const btree_t *tree);

// 遍历B树，对每个关键字调用指定函数
extern void btree_foreach(const btree_t *tree, 
                         void (*callback)(void *key, void *arg),
                         void *arg);

// 中序遍历B树，以有序方式检索所有关键字
extern void btree_inorder(const btree_t *tree,
                         void (*callback)(void *key, void *arg),
                         void *arg);

#endif // __B_TREE_H__