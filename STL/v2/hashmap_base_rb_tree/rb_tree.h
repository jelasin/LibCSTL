#ifndef __RB_TREE_H__
#define __RB_TREE_H__

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

#ifndef rb_entry
#define rb_entry(ptr, type, member) \
    container_of(ptr, type, member)
#endif

// 红黑树节点颜色
enum rb_color {
    RB_BLACK,
    RB_RED
};

// 红黑树节点
struct rb_node {
    struct rb_node *parent;     // 父节点
    struct rb_node *left;       // 左子节点
    struct rb_node *right;      // 右子节点
    enum rb_color color;        // 节点颜色
};

// 红黑树根
typedef struct rb_root {
    struct rb_node *root;       // 根节点
    
    // 比较函数，返回值：
    // < 0: a < b
    // = 0: a == b
    // > 0: a > b
    int (*compare)(const struct rb_node *a, 
                  const struct rb_node *b, void *arg);
    void *compare_arg;          // 比较函数参数
    
    // 析构函数，用于释放节点内存（如果需要）
    void (*node_destructor)(struct rb_node *node, void *arg);
    void *destructor_arg;       // 析构函数参数
} rb_root_t;

// 初始化红黑树
extern void rb_init(rb_root_t *tree, 
                   int (*compare)(const struct rb_node *a, 
                                 const struct rb_node *b, void *arg),
                   void *compare_arg,
                   void (*node_destructor)(struct rb_node *node, void *arg),
                   void *destructor_arg);

// 查找节点
extern struct rb_node *rb_search(const rb_root_t *tree, const struct rb_node *key);

// 插入节点
extern int rb_insert(rb_root_t *tree, struct rb_node *node);

// 删除节点
extern void rb_erase(rb_root_t *tree, struct rb_node *node);

// 获取最小值节点
extern struct rb_node *rb_first(const rb_root_t *tree);

// 获取最大值节点
extern struct rb_node *rb_last(const rb_root_t *tree);

// 获取后继节点
extern struct rb_node *rb_next(const struct rb_node *node);

// 获取前驱节点
extern struct rb_node *rb_prev(const struct rb_node *node);

// 判断红黑树是否为空
extern int rb_empty(const rb_root_t *tree);

// 清空红黑树
extern void rb_clear(rb_root_t *tree);

// 销毁红黑树（递归释放所有节点内存）
extern void rb_destroy(rb_root_t *tree);

// 替换红黑树节点
extern void rb_replace(rb_root_t *tree, 
                      struct rb_node *old_node, 
                      struct rb_node *new_node);

// 遍历红黑树的宏（中序遍历）
#define rb_inorder(pos, tree, type, member) \
    for (pos = rb_entry(rb_first(tree), type, member); \
         &pos->member != NULL; \
         pos = rb_entry(rb_next(&pos->member), type, member))

// 验证红黑树合法性（调试用）
extern int rb_verify(const rb_root_t *tree);

#endif // __RB_TREE_H__