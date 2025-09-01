/*
 * Linux内核风格红黑树实现
 * 完全基于 Linux Kernel lib/rbtree.c
 * 
 * 红黑树性质：
 * 1. 每个节点不是红色就是黑色
 * 2. 根节点是黑色的
 * 3. 每个叶子节点（NIL）是黑色的
 * 4. 如果一个节点是红色的，则它的两个子节点都是黑色的
 * 5. 对于每个节点，从该节点到其所有后代叶子节点的简单路径上包含相同数目的黑色节点
 */

#include "rb_tree.h"

#ifndef true
#define true 1
#define false 0
#endif

/*
 * 红黑树辅助函数
 */

static inline void __rb_change_child(struct rb_node *old, struct rb_node *new_node,
                                    struct rb_node *parent, struct rb_root *root)
{
    if (parent) {
        if (parent->rb_left == old)
            parent->rb_left = new_node;
        else
            parent->rb_right = new_node;
    } else {
        root->rb_node = new_node;
    }
}

static inline void __rb_rotate_set_parents(struct rb_node *old, struct rb_node *new_node,
                                         struct rb_root *root, int color)
{
    struct rb_node *parent = rb_parent(old);
    new_node->__rb_parent_color = old->__rb_parent_color;
    rb_set_parent_color(old, new_node, color);
    __rb_change_child(old, new_node, parent, root);
}

/*
 * 插入修复函数
 */
void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *parent = rb_parent(node), *gparent, *tmp;

    while (true) {
        if (!parent) {
            /* 情况1：插入的是根节点，设为黑色 */
            rb_set_parent_color(node, NULL, RB_BLACK);
            break;
        } else if (rb_is_black(parent)) {
            /* 情况2：父节点是黑色，不违反红黑树性质 */
            break;
        }

        gparent = rb_parent(parent);

        if (parent == gparent->rb_left) {
            tmp = gparent->rb_right;
            if (tmp && rb_is_red(tmp)) {
                /* 情况3：叔节点是红色 */
                rb_set_parent_color(tmp, gparent, RB_BLACK);
                rb_set_parent_color(parent, gparent, RB_BLACK);
                node = gparent;
                parent = rb_parent(node);
                rb_set_parent_color(node, parent, RB_RED);
                continue;
            }

            if (node == parent->rb_right) {
                /* 情况4：当前节点是父节点的右子节点 */
                tmp = node->rb_left;
                parent->rb_right = tmp;
                node->rb_left = parent;
                if (tmp)
                    rb_set_parent_color(tmp, parent, RB_BLACK);
                rb_set_parent_color(parent, node, RB_RED);
                parent = node;
            }

            /* 情况5：当前节点是父节点的左子节点 */
            gparent->rb_left = parent->rb_right;
            parent->rb_right = gparent;
            if (gparent->rb_left)
                rb_set_parent_color(gparent->rb_left, gparent, RB_BLACK);
            __rb_rotate_set_parents(gparent, parent, root, RB_RED);
            break;
        } else {
            tmp = gparent->rb_left;
            if (tmp && rb_is_red(tmp)) {
                /* 情况3的镜像 */
                rb_set_parent_color(tmp, gparent, RB_BLACK);
                rb_set_parent_color(parent, gparent, RB_BLACK);
                node = gparent;
                parent = rb_parent(node);
                rb_set_parent_color(node, parent, RB_RED);
                continue;
            }

            if (node == parent->rb_left) {
                /* 情况4的镜像 */
                tmp = node->rb_right;
                parent->rb_left = tmp;
                node->rb_right = parent;
                if (tmp)
                    rb_set_parent_color(tmp, parent, RB_BLACK);
                rb_set_parent_color(parent, node, RB_RED);
                parent = node;
            }

            /* 情况5的镜像 */
            gparent->rb_right = parent->rb_left;
            parent->rb_left = gparent;
            if (gparent->rb_right)
                rb_set_parent_color(gparent->rb_right, gparent, RB_BLACK);
            __rb_rotate_set_parents(gparent, parent, root, RB_RED);
            break;
        }
    }
}

/*
 * 删除修复函数 - 直接来自Linux内核
 */
static void ____rb_erase_color(struct rb_node *parent, struct rb_root *root,
                             void (*augment_rotate)(struct rb_node *old, struct rb_node *new_node))
{
    struct rb_node *node = NULL, *sibling, *tmp1, *tmp2;

    while (true) {
        sibling = parent->rb_right;
        if (node != sibling) {    /* node == parent->rb_left */
            if (rb_is_red(sibling)) {
                /* 情况1：兄弟节点是红色 */
                tmp1 = sibling->rb_left;
                parent->rb_right = tmp1;
                sibling->rb_left = parent;
                if (tmp1)
                    rb_set_parent_color(tmp1, parent, RB_BLACK);
                __rb_rotate_set_parents(parent, sibling, root, RB_RED);
                if (augment_rotate)
                    augment_rotate(parent, sibling);
                sibling = tmp1;
            }
            if (!sibling) break;  // 安全检查：如果sibling为NULL则退出
            tmp1 = sibling->rb_right;
            if (!tmp1 || rb_is_black(tmp1)) {
                tmp2 = sibling->rb_left;
                if (!tmp2 || rb_is_black(tmp2)) {
                    /* 情况2：兄弟节点的两个子节点都是黑色 */
                    rb_set_parent_color(sibling, parent, RB_RED);
                    if (rb_is_red(parent))
                        rb_set_black(parent);
                    else {
                        node = parent;
                        parent = rb_parent(node);
                        if (parent)
                            continue;
                    }
                    break;
                }
                /* 情况3：兄弟节点的左子节点是红色，右子节点是黑色 */
                tmp1 = tmp2->rb_right;
                sibling->rb_left = tmp1;
                tmp2->rb_right = sibling;
                parent->rb_right = tmp2;
                if (tmp1)
                    rb_set_parent_color(tmp1, sibling, RB_BLACK);
                if (augment_rotate)
                    augment_rotate(sibling, tmp2);
                tmp1 = sibling;
                sibling = tmp2;
            }
            /* 情况4：兄弟节点的右子节点是红色 */
            tmp2 = sibling->rb_left;
            parent->rb_right = tmp2;
            sibling->rb_left = parent;
            rb_set_parent_color(tmp1, sibling, RB_BLACK);
            if (tmp2)
                rb_set_parent(tmp2, parent);
            __rb_rotate_set_parents(parent, sibling, root, RB_BLACK);
            if (augment_rotate)
                augment_rotate(parent, sibling);
            break;
        } else {
            sibling = parent->rb_left;
            if (rb_is_red(sibling)) {
                /* 情况1的镜像 */
                tmp1 = sibling->rb_right;
                parent->rb_left = tmp1;
                sibling->rb_right = parent;
                if (tmp1)
                    rb_set_parent_color(tmp1, parent, RB_BLACK);
                __rb_rotate_set_parents(parent, sibling, root, RB_RED);
                if (augment_rotate)
                    augment_rotate(parent, sibling);
                sibling = tmp1;
            }
            if (!sibling) break;  // 安全检查：如果sibling为NULL则退出
            tmp1 = sibling->rb_left;
            if (!tmp1 || rb_is_black(tmp1)) {
                tmp2 = sibling->rb_right;
                if (!tmp2 || rb_is_black(tmp2)) {
                    /* 情况2的镜像 */
                    rb_set_parent_color(sibling, parent, RB_RED);
                    if (rb_is_red(parent))
                        rb_set_black(parent);
                    else {
                        node = parent;
                        parent = rb_parent(node);
                        if (parent)
                            continue;
                    }
                    break;
                }
                /* 情况3的镜像 */
                tmp1 = tmp2->rb_left;
                sibling->rb_right = tmp1;
                tmp2->rb_left = sibling;
                parent->rb_left = tmp2;
                if (tmp1)
                    rb_set_parent_color(tmp1, sibling, RB_BLACK);
                if (augment_rotate)
                    augment_rotate(sibling, tmp2);
                tmp1 = sibling;
                sibling = tmp2;
            }
            /* 情况4的镜像 */
            tmp2 = sibling->rb_right;
            parent->rb_left = tmp2;
            sibling->rb_right = parent;
            rb_set_parent_color(tmp1, sibling, RB_BLACK);
            if (tmp2)
                rb_set_parent(tmp2, parent);
            __rb_rotate_set_parents(parent, sibling, root, RB_BLACK);
            if (augment_rotate)
                augment_rotate(parent, sibling);
            break;
        }
    }
}

static struct rb_node *__rb_erase_augmented(struct rb_node *node, struct rb_root *root,
                                          void (*augment_rotate)(struct rb_node *old, struct rb_node *new_node))
{
    struct rb_node *child = node->rb_right;
    struct rb_node *tmp = node->rb_left;
    struct rb_node *parent, *rebalance;
    unsigned long pc;

    if (!tmp) {
        /* 情况1：节点最多有一个右子节点 */
        pc = node->__rb_parent_color;
        parent = __builtin_expect(pc & ~3, 0) ? (struct rb_node *)(pc & ~3) : NULL;
        __rb_change_child(node, child, parent, root);
        if (child) {
            child->__rb_parent_color = pc;
            rebalance = NULL;
        } else
            rebalance = __builtin_expect(pc & 1, 1) ? NULL : parent;
        tmp = parent;
    } else if (!child) {
        /* 情况2：节点只有一个左子节点 */
        tmp->__rb_parent_color = pc = node->__rb_parent_color;
        parent = __builtin_expect(pc & ~3, 0) ? (struct rb_node *)(pc & ~3) : NULL;
        __rb_change_child(node, tmp, parent, root);
        rebalance = NULL;
        tmp = parent;
    } else {
        struct rb_node *successor = child, *child2;

        tmp = child->rb_left;
        if (!tmp) {
            /* 情况3：节点的右子节点没有左子节点，右子节点就是后继节点 */
            parent = successor;
            child2 = successor->rb_right;
            if (augment_rotate)
                augment_rotate(node, successor);
        } else {
            /* 情况4：找到后继节点（右子树中最小的节点） */
            do {
                parent = successor;
                successor = tmp;
                tmp = tmp->rb_left;
            } while (tmp);
            child2 = successor->rb_right;
            parent->rb_left = child2;
            successor->rb_right = child;
            rb_set_parent(child, successor);
            if (augment_rotate) {
                augment_rotate(node, successor);
                augment_rotate(parent, successor);
            }
        }

        tmp = node->rb_left;
        successor->rb_left = tmp;
        rb_set_parent(tmp, successor);

        pc = node->__rb_parent_color;
        tmp = __builtin_expect(pc & ~3, 0) ? (struct rb_node *)(pc & ~3) : NULL;
        __rb_change_child(node, successor, tmp, root);

        if (child2) {
            successor->__rb_parent_color = pc;
            rb_set_parent_color(child2, parent, RB_BLACK);
            rebalance = NULL;
        } else {
            unsigned long pc2 = successor->__rb_parent_color;
            successor->__rb_parent_color = pc;
            rebalance = __builtin_expect(pc2 & 1, 1) ? NULL : parent;
        }
        tmp = successor;
    }

    if (augment_rotate && tmp)
        augment_rotate(tmp, rebalance);
    return rebalance;
}

void __rb_erase(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *rebalance;
    rebalance = __rb_erase_augmented(node, root, NULL);
    if (rebalance)
        ____rb_erase_color(rebalance, root, NULL);
}

/*
 * 查找函数实现
 */

struct rb_node *rb_first(const struct rb_root *root)
{
    struct rb_node *n;

    n = root->rb_node;
    if (!n)
        return NULL;
    while (n->rb_left)
        n = n->rb_left;
    return n;
}

struct rb_node *rb_last(const struct rb_root *root)
{
    struct rb_node *n;

    n = root->rb_node;
    if (!n)
        return NULL;
    while (n->rb_right)
        n = n->rb_right;
    return n;
}

struct rb_node *rb_next(const struct rb_node *node)
{
    struct rb_node *parent;

    if (RB_EMPTY_NODE(node))
        return NULL;

    /* 如果有右子树，则后继是右子树的最小节点 */
    if (node->rb_right) {
        node = node->rb_right;
        while (node->rb_left)
            node = node->rb_left;
        return (struct rb_node *)node;
    }

    /* 没有右子树：向上找到第一个是其父节点左子节点的节点 */
    while ((parent = rb_parent(node)) && node == parent->rb_right)
        node = parent;

    return parent;
}

struct rb_node *rb_prev(const struct rb_node *node)
{
    struct rb_node *parent;

    if (RB_EMPTY_NODE(node))
        return NULL;

    /* 如果有左子树，则前驱是左子树的最大节点 */
    if (node->rb_left) {
        node = node->rb_left;
        while (node->rb_right)
            node = node->rb_right;
        return (struct rb_node *)node;
    }

    /* 没有左子树：向上找到第一个是其父节点右子节点的节点 */
    while ((parent = rb_parent(node)) && node == parent->rb_left)
        node = parent;

    return parent;
}

void rb_replace_node(struct rb_node *victim, struct rb_node *new_node,
                    struct rb_root *root)
{
    struct rb_node *parent = rb_parent(victim);

    /* 复制颜色和父节点信息 */
    new_node->__rb_parent_color = victim->__rb_parent_color;
    new_node->rb_left = victim->rb_left;
    new_node->rb_right = victim->rb_right;

    /* 更新父节点 */
    __rb_change_child(victim, new_node, parent, root);

    /* 更新子节点的父指针 */
    if (victim->rb_left)
        rb_set_parent(victim->rb_left, new_node);
    if (victim->rb_right)
        rb_set_parent(victim->rb_right, new_node);
}

/* 后序遍历相关函数 */
struct rb_node *rb_first_postorder_cached(const struct rb_root *root,
                                         struct rb_node **cache)
{
    if (!root->rb_node)
        return NULL;

    struct rb_node *node = root->rb_node;
    while (true) {
        if (node->rb_left) {
            node = node->rb_left;
        } else if (node->rb_right) {
            node = node->rb_right;
        } else {
            return (struct rb_node *)node;
        }
    }
}

struct rb_node *rb_next_postorder_cached(const struct rb_node *node,
                                        struct rb_node **cache)
{
    struct rb_node *parent = rb_parent(node);
    
    if (!parent)
        return NULL;
        
    if (node == parent->rb_left && parent->rb_right) {
        /* 从左子树返回，继续右子树 */
        node = parent->rb_right;
        while (true) {
            if (node->rb_left) {
                node = node->rb_left;
            } else if (node->rb_right) {
                node = node->rb_right;
            } else {
                return (struct rb_node *)node;
            }
        }
    }
    
    return parent;
}

/*
 * 兼容性API实现
 */

void rb_init(rb_root_t *tree, 
             int (*compare)(const struct rb_node *a, 
                           const struct rb_node *b, void *arg),
             void *compare_arg,
             void (*node_destructor)(struct rb_node *node, void *arg),
             void *destructor_arg) 
{
    tree->root.rb_node = NULL;
    tree->compare = compare;
    tree->compare_arg = compare_arg;
    tree->node_destructor = node_destructor;
    tree->destructor_arg = destructor_arg;
}

struct rb_node *rb_search(const rb_root_t *tree, const struct rb_node *key)
{
    struct rb_node *node = tree->root.rb_node;
    
    while (node) {
        int result = tree->compare(key, node, tree->compare_arg);
        
        if (result < 0) {
            node = node->rb_left;
        } else if (result > 0) {
            node = node->rb_right;
        } else {
            return node;  /* 找到匹配的节点 */
        }
    }
    
    return NULL;  /* 未找到 */
}

int rb_insert(rb_root_t *tree, struct rb_node *node)
{
    struct rb_node **new_node = &(tree->root.rb_node), *parent = NULL;
    
    /* 查找插入位置 */
    while (*new_node) {
        int result = tree->compare(node, *new_node, tree->compare_arg);
        
        parent = *new_node;
        if (result < 0) {
            new_node = &((*new_node)->rb_left);
        } else if (result > 0) {
            new_node = &((*new_node)->rb_right);
        } else {
            return -1;  /* 节点已存在 */
        }
    }
    
    /* 链接新节点 */
    rb_link_node(node, parent, new_node);
    rb_insert_color(node, &tree->root);
    
    return 0;
}

void rb_erase(rb_root_t *tree, struct rb_node *node)
{
    __rb_erase(node, &tree->root);
}

int rb_empty(const rb_root_t *tree)
{
    return RB_EMPTY_ROOT(&tree->root);
}

static void rb_destroy_recursive(rb_root_t *tree, struct rb_node *node)
{
    if (!node) return;
    
    /* 递归销毁左右子树 */
    rb_destroy_recursive(tree, node->rb_left);
    rb_destroy_recursive(tree, node->rb_right);
    
    /* 调用析构函数释放节点 */
    if (tree->node_destructor) {
        tree->node_destructor(node, tree->destructor_arg);
    }
}

void rb_clear(rb_root_t *tree)
{
    rb_destroy_recursive(tree, tree->root.rb_node);
    tree->root.rb_node = NULL;
}

void rb_destroy(rb_root_t *tree)
{
    rb_clear(tree);
}

void rb_replace(rb_root_t *tree, 
               struct rb_node *old_node, 
               struct rb_node *new_node)
{
    rb_replace_node(old_node, new_node, &tree->root);
}

/*
 * 红黑树验证函数（调试用）
 */

static int rb_black_height(const struct rb_node *node)
{
    int left_height, right_height;
    
    if (!node) return 1;  /* NULL节点是黑色的 */
    
    left_height = rb_black_height(node->rb_left);
    right_height = rb_black_height(node->rb_right);
    
    if (left_height == 0 || right_height == 0 || left_height != right_height) {
        return 0;  /* 违反红黑树性质 */
    }
    
    if (rb_is_black(node)) {
        return left_height + 1;
    } else {
        return left_height;
    }
}

static int rb_verify_node(const struct rb_node *node)
{
    if (!node) return 1;
    
    /* 检查红色节点的子节点必须是黑色 */
    if (rb_is_red(node)) {
        if (node->rb_left && rb_is_red(node->rb_left)) return 0;
        if (node->rb_right && rb_is_red(node->rb_right)) return 0;
    }
    
    /* 检查父子关系 */
    if (node->rb_left && rb_parent(node->rb_left) != node) return 0;
    if (node->rb_right && rb_parent(node->rb_right) != node) return 0;
    
    /* 递归验证子树 */
    return rb_verify_node(node->rb_left) && rb_verify_node(node->rb_right);
}

int rb_verify(const rb_root_t *tree)
{
    struct rb_node *root = tree->root.rb_node;
    
    if (!root) return 1;  /* 空树是合法的 */
    
    /* 根节点必须是黑色 */
    if (rb_is_red(root)) return 0;
    
    /* 检查黑色高度平衡 */
    if (rb_black_height(root) == 0) return 0;
    
    /* 检查节点性质 */
    return rb_verify_node(root);
}
