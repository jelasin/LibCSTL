#ifndef __RB_TREE_H__
#define __RB_TREE_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef offsetof
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

/*
 * Linux内核风格红黑树实现
 * 参考 include/linux/rbtree.h 和 lib/rbtree.c
 * 
 * 主要特性：
 * 1. 颜色信息存储在父指针的最低位，节省内存
 * 2. 提供原始的低级接口和易用的高级接口
 * 3. 支持中序遍历的宏定义
 * 4. 自平衡红黑树，保证 O(log n) 时间复杂度
 */

// 红黑树节点颜色常量
#define RB_RED      0
#define RB_BLACK    1

// 红黑树节点结构（Linux内核风格）
struct rb_node {
    unsigned long  __rb_parent_color;   // 父节点指针+颜色位
    struct rb_node *rb_right;           // 右子节点
    struct rb_node *rb_left;            // 左子节点
} __attribute__((aligned(sizeof(long))));

// 红黑树根节点
struct rb_root {
    struct rb_node *rb_node;
};

// 静态初始化红黑树根
#define RB_ROOT     (struct rb_root) { NULL, }

// 颜色和父节点操作宏
#define rb_parent(r)    ((struct rb_node *)((r)->__rb_parent_color & ~3))
#define rb_color(r)     ((r)->__rb_parent_color & 1)
#define rb_is_red(r)    (!rb_color(r))
#define rb_is_black(r)  rb_color(r)
#define rb_set_red(r)   do { (r)->__rb_parent_color &= ~1; } while (0)
#define rb_set_black(r) do { (r)->__rb_parent_color |= 1; } while (0)

// 设置父节点和颜色
static inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
    rb->__rb_parent_color = rb_color(rb) | (unsigned long)p;
}

static inline void rb_set_parent_color(struct rb_node *rb,
                                      struct rb_node *p, int color)
{
    rb->__rb_parent_color = (unsigned long)p | color;
}

// 红黑树遍历宏（提前定义）
#define rb_first_postorder(root) rb_first_postorder_cached(root, NULL)
#define rb_next_postorder(node)  rb_next_postorder_cached(node, NULL)

/*
 * 高级API：兼容原有接口的包装层
 * 这些API提供了更易用的接口，保持向后兼容性
 */

// 兼容性红黑树根结构
typedef struct rb_root_compat {
    struct rb_root root;            // Linux内核风格根节点
    
    // 比较函数，返回值：
    // < 0: a < b
    // = 0: a == b  
    // > 0: a > b
    int (*compare)(const struct rb_node *a, 
                  const struct rb_node *b, void *arg);
    void *compare_arg;              // 比较函数参数
    
    // 析构函数，用于释放节点内存（如果需要）
    void (*node_destructor)(struct rb_node *node, void *arg);
    void *destructor_arg;           // 析构函数参数
} rb_root_compat_t;

// 为了向后兼容，保留原有的类型名
typedef rb_root_compat_t rb_root_t;

/*
 * Linux内核风格的核心API（低级接口）
 */

// 初始化红黑树节点
static inline void rb_init_node(struct rb_node *rb)
{
    rb->__rb_parent_color = 0;
    rb->rb_left = NULL;
    rb->rb_right = NULL;
}

// 插入节点
extern void rb_insert_color(struct rb_node *, struct rb_root *);

// 查找第一个节点（最小值）
extern struct rb_node *rb_first(const struct rb_root *);

// 查找最后一个节点（最大值）  
extern struct rb_node *rb_last(const struct rb_root *);

// 查找下一个节点（中序遍历）
extern struct rb_node *rb_next(const struct rb_node *);

// 查找上一个节点（中序遍历）
extern struct rb_node *rb_prev(const struct rb_node *);

// 替换节点（保持树结构）
extern void rb_replace_node(struct rb_node *victim, struct rb_node *new_node,
                           struct rb_root *root);

// 后序遍历相关
extern struct rb_node *rb_first_postorder_cached(const struct rb_root *root,
                                                 struct rb_node **cache);

extern struct rb_node *rb_next_postorder_cached(const struct rb_node *node,
                                               struct rb_node **cache);

/*
 * 遍历宏（Linux内核风格）
 */

// 中序遍历（从小到大）
#define rbtree_postorder_for_each_entry_safe(pos, n, root, field) \
    for (pos = rb_entry_safe(rb_first_postorder(root), typeof(*pos), field); \
         pos && ({ n = rb_entry_safe(rb_next_postorder(&pos->field), \
                                   typeof(*pos), field); 1; }); \
         pos = n)

// 安全的rb_entry宏
#define rb_entry_safe(ptr, type, member) \
    ({ typeof(ptr) ____ptr = (ptr); \
       ____ptr ? rb_entry(____ptr, type, member) : NULL; })

/*
 * 高级API：兼容原有接口的包装层
 */

// 初始化红黑树（兼容性接口）
extern void rb_init(rb_root_t *tree, 
                   int (*compare)(const struct rb_node *a, 
                                 const struct rb_node *b, void *arg),
                   void *compare_arg,
                   void (*node_destructor)(struct rb_node *node, void *arg),
                   void *destructor_arg);

// 查找节点（兼容性接口）
extern struct rb_node *rb_search(const rb_root_t *tree, const struct rb_node *key);

// 插入节点（兼容性接口）
extern int rb_insert(rb_root_t *tree, struct rb_node *node);

// 删除节点（兼容性接口） 
extern void rb_erase(rb_root_t *tree, struct rb_node *node);

// 判断红黑树是否为空
extern int rb_empty(const rb_root_t *tree);

// 清空红黑树
extern void rb_clear(rb_root_t *tree);

// 销毁红黑树（递归释放所有节点内存）
extern void rb_destroy(rb_root_t *tree);

// 替换红黑树节点（兼容性接口）
extern void rb_replace(rb_root_t *tree, 
                      struct rb_node *old_node, 
                      struct rb_node *new_node);

// 遍历红黑树的宏（中序遍历，兼容性接口）
#define rb_inorder(pos, tree, type, member) \
    for (pos = rb_entry_safe(rb_first(&(tree)->root), type, member); \
         pos != NULL; \
         pos = rb_entry_safe(rb_next(&pos->member), type, member))

// 验证红黑树合法性（调试用）
extern int rb_verify(const rb_root_t *tree);

/*
 * 内联辅助函数
 */

// 获取红黑树根节点
static inline int RB_EMPTY_ROOT(const struct rb_root *root)
{
    return root->rb_node == NULL;
}

// 判断节点是否为空
static inline int RB_EMPTY_NODE(const struct rb_node *node)
{
    return rb_parent(node) == node;
}

// 清空节点
static inline void RB_CLEAR_NODE(struct rb_node *node)
{
    rb_set_parent(node, node);
}

// 复制颜色
static inline void rb_set_color(struct rb_node *rb, int color)
{
    rb->__rb_parent_color = (rb->__rb_parent_color & ~1) | color;
}

// 链接新节点到父节点
static inline void rb_link_node(struct rb_node *node, struct rb_node *parent,
                               struct rb_node **rb_link)
{
    node->__rb_parent_color = (unsigned long)parent;
    node->rb_left = node->rb_right = NULL;
    
    *rb_link = node;
}

#endif // __RB_TREE_H__