#ifndef __SPLAY_TREE_H__
#define __SPLAY_TREE_H__

#include <stdlib.h>

#ifndef offsetof
typedef unsigned long size_t;
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({          \
        const typeof( ((type *)0)->member ) *__mptr = (const typeof( ((type *)0)->member ) *)(ptr); \
        (type *)( (char *)__mptr - offsetof(type, member) );})
#endif

#ifndef splay_entry
#define splay_entry(ptr, type, member) \
    container_of(ptr, type, member)
#endif

// 伸展树节点
struct splay_node {
    struct splay_node *left;    // 左子节点
    struct splay_node *right;   // 右子节点
    struct splay_node *parent;  // 父节点
};

// 伸展树根
typedef struct splay_root {
    struct splay_node *root;    // 根节点
    
    // 比较函数，返回值:
    // < 0: a < b
    // = 0: a == b
    // > 0: a > b
    int (*compare)(const struct splay_node *a, const struct splay_node *b, void *arg);
    void *compare_arg;          // 比较函数的额外参数
    
    // 析构函数，用于释放节点内存（如果需要）
    void (*node_destructor)(struct splay_node *node, void *arg);
    void *destructor_arg;       // 析构函数参数
} splay_root_t;

// 初始化伸展树
extern void splay_init(splay_root_t *tree, 
                       int (*compare)(const struct splay_node *a, 
                                      const struct splay_node *b, void *arg),
                       void *compare_arg,
                       void (*node_destructor)(struct splay_node *node, void *arg),
                       void *destructor_arg);

// 查找节点（会将找到的节点伸展到根部）
extern struct splay_node *splay_search(splay_root_t *tree, struct splay_node *key);

// 插入节点
extern int splay_insert(splay_root_t *tree, struct splay_node *node);

// 删除节点
extern void splay_erase(splay_root_t *tree, struct splay_node *node);

// 获取最小值节点
extern struct splay_node *splay_first(const splay_root_t *tree);

// 获取最大值节点
extern struct splay_node *splay_last(const splay_root_t *tree);

// 获取后继节点
extern struct splay_node *splay_next(const struct splay_node *node);

// 获取前驱节点
extern struct splay_node *splay_prev(const struct splay_node *node);

// 判断伸展树是否为空
extern int splay_empty(const splay_root_t *tree);

// 清空伸展树（不释放节点内存）
extern void splay_clear(splay_root_t *tree);

// 销毁伸展树（递归释放所有节点内存）
extern void splay_destroy(splay_root_t *tree);

// 替换伸展树根节点
extern void splay_replace(splay_root_t *tree, 
                         struct splay_node *old_node, 
                         struct splay_node *new_node);

// 中序遍历宏（不会修改树结构）
#define splay_inorder(pos, tree, type, member) \
    for (pos = splay_entry(splay_first(tree), type, member); \
         &pos->member != NULL; \
         pos = splay_entry(splay_next(&pos->member), type, member))

#endif // __SPLAY_TREE_H__