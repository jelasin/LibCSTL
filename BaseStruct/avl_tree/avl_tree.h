#ifndef __AVL_TREE_H__
#define __AVL_TREE_H__

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

#ifndef avl_entry
#define avl_entry(ptr, type, member) \
    container_of(ptr, type, member)
#endif

// AVL树节点
struct avl_node {
    struct avl_node *left;     // 左子节点
    struct avl_node *right;    // 右子节点
    struct avl_node *parent;   // 父节点
    int height;                // 节点高度
};

// AVL树根
typedef struct avl_root {
    struct avl_node *root;     // 根节点
    
    // 比较函数，返回值:
    // < 0: a < b
    // = 0: a == b
    // > 0: a > b
    int (*compare)(const struct avl_node *a, const struct avl_node *b, void *arg);
    void *compare_arg;         // 比较函数的额外参数
    
    // 析构函数，用于释放节点内存（如果需要）
    void (*node_destructor)(struct avl_node *node, void *arg);
    void *destructor_arg;      // 析构函数参数
} avl_root_t;

// 初始化AVL树
extern void avl_init(avl_root_t *tree, 
                    int (*compare)(const struct avl_node *a, 
                                  const struct avl_node *b, void *arg),
                    void *compare_arg,
                    void (*node_destructor)(struct avl_node *node, void *arg),
                    void *destructor_arg);

// 查找节点
extern struct avl_node *avl_search(const avl_root_t *tree, const struct avl_node *key);

// 插入节点
extern int avl_insert(avl_root_t *tree, struct avl_node *node);

// 删除节点
extern void avl_erase(avl_root_t *tree, struct avl_node *node);

// 获取最小值节点
extern struct avl_node *avl_first(const avl_root_t *tree);

// 获取最大值节点
extern struct avl_node *avl_last(const avl_root_t *tree);

// 获取后继节点
extern struct avl_node *avl_next(const struct avl_node *node);

// 获取前驱节点
extern struct avl_node *avl_prev(const struct avl_node *node);

// 判断AVL树是否为空
extern int avl_empty(const avl_root_t *tree);

// 销毁AVL树（递归释放所有节点内存）
extern void avl_destroy(avl_root_t *tree);

// 替换AVL树节点
extern void avl_replace(avl_root_t *tree, 
                      struct avl_node *old_node, 
                      struct avl_node *new_node);

// 获取节点高度
extern int avl_height(const struct avl_node *node);

// 获取节点平衡因子
extern int avl_balance_factor(const struct avl_node *node);

// 中序遍历宏（不会修改树结构）
#define avl_inorder(pos, tree, type, member) \
    for (struct avl_node *__cur = avl_first(tree); \
         __cur && ((pos) = avl_entry(__cur, type, member), 1); \
         __cur = avl_next(__cur))

#endif // __AVL_TREE_H__