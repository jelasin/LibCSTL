#include "rb_tree.h"

// 初始化红黑树
void rb_init(rb_root_t *tree, 
             int (*compare)(const struct rb_node *a, 
                           const struct rb_node *b, void *arg),
             void *compare_arg,
             void (*node_destructor)(struct rb_node *node, void *arg),
             void *destructor_arg) 
{
    tree->root = NULL;
    tree->compare = compare;
    tree->compare_arg = compare_arg;
    tree->node_destructor = node_destructor;
    tree->destructor_arg = destructor_arg;
}

// 左旋转
static void rb_rotate_left(rb_root_t *tree, struct rb_node *node) 
{
    struct rb_node *right = node->right;
    struct rb_node *parent = node->parent;
    
    // 节点的右子节点变为右子节点的左子节点
    node->right = right->left;
    if (right->left)
        right->left->parent = node;
    
    // 右子节点的父节点变为节点的父节点
    right->parent = parent;
    
    // 更新父节点的子节点
    if (!parent) {
        tree->root = right;
    } else if (node == parent->left) {
        parent->left = right;
    } else {
        parent->right = right;
    }
    
    // 节点成为右子节点的左子节点
    right->left = node;
    node->parent = right;
}

// 右旋转
static void rb_rotate_right(rb_root_t *tree, struct rb_node *node) 
{
    struct rb_node *left = node->left;
    struct rb_node *parent = node->parent;
    
    // 节点的左子节点变为左子节点的右子节点
    node->left = left->right;
    if (left->right)
        left->right->parent = node;
    
    // 左子节点的父节点变为节点的父节点
    left->parent = parent;
    
    // 更新父节点的子节点
    if (!parent) {
        tree->root = left;
    } else if (node == parent->left) {
        parent->left = left;
    } else {
        parent->right = left;
    }
    
    // 节点成为左子节点的右子节点
    left->right = node;
    node->parent = left;
}

// 插入修复
static void rb_insert_fixup(rb_root_t *tree, struct rb_node *node) 
{
    struct rb_node *parent, *gparent, *uncle;
    
    // 当父节点存在且为红色时需要修复
    while ((parent = node->parent) && parent->color == RB_RED) {
        gparent = parent->parent;
        
        if (parent == gparent->left) {
            uncle = gparent->right;
            
            // 情况1：叔叔节点是红色
            if (uncle && uncle->color == RB_RED) {
                uncle->color = RB_BLACK;
                parent->color = RB_BLACK;
                gparent->color = RB_RED;
                node = gparent;
                continue;
            }
            
            // 情况2：叔叔节点是黑色，当前节点是右子节点
            if (node == parent->right) {
                rb_rotate_left(tree, parent);
                struct rb_node *tmp = parent;
                parent = node;
                node = tmp;
            }
            
            // 情况3：叔叔节点是黑色，当前节点是左子节点
            parent->color = RB_BLACK;
            gparent->color = RB_RED;
            rb_rotate_right(tree, gparent);
        } else {
            uncle = gparent->left;
            
            // 情况1：叔叔节点是红色
            if (uncle && uncle->color == RB_RED) {
                uncle->color = RB_BLACK;
                parent->color = RB_BLACK;
                gparent->color = RB_RED;
                node = gparent;
                continue;
            }
            
            // 情况2：叔叔节点是黑色，当前节点是左子节点
            if (node == parent->left) {
                rb_rotate_right(tree, parent);
                struct rb_node *tmp = parent;
                parent = node;
                node = tmp;
            }
            
            // 情况3：叔叔节点是黑色，当前节点是右子节点
            parent->color = RB_BLACK;
            gparent->color = RB_RED;
            rb_rotate_left(tree, gparent);
        }
    }
    
    // 确保根节点是黑色
    tree->root->color = RB_BLACK;
}

// 插入节点
int rb_insert(rb_root_t *tree, struct rb_node *node) 
{
    struct rb_node *current = tree->root;
    struct rb_node *parent = NULL;
    int cmp;
    
    // 初始化新节点
    node->left = NULL;
    node->right = NULL;
    node->color = RB_RED;
    
    // 处理空树情况
    if (!current) {
        node->parent = NULL;
        node->color = RB_BLACK;
        tree->root = node;
        return 0;
    }
    
    // 查找插入位置
    while (current) {
        parent = current;
        cmp = tree->compare(node, current, tree->compare_arg);
        
        if (cmp < 0)
            current = current->left;
        else if (cmp > 0)
            current = current->right;
        else
            return -1;  // 节点已存在
    }
    
    // 插入新节点
    node->parent = parent;
    if (cmp < 0)
        parent->left = node;
    else
        parent->right = node;
    
    // 修复红黑树性质
    rb_insert_fixup(tree, node);
    
    return 0;
}

// 查找节点
struct rb_node *rb_search(const rb_root_t *tree, const struct rb_node *key) 
{
    struct rb_node *current = tree->root;
    int cmp;
    
    while (current) {
        cmp = tree->compare(key, current, tree->compare_arg);
        
        if (cmp < 0)
            current = current->left;
        else if (cmp > 0)
            current = current->right;
        else
            return current;  // 找到节点
    }
    
    return NULL;  // 未找到节点
}

// 获取最小值节点
struct rb_node *rb_first(const rb_root_t *tree) 
{
    struct rb_node *node = tree->root;
    
    if (!node)
        return NULL;
    
    while (node->left)
        node = node->left;
    
    return node;
}

// 获取最大值节点
struct rb_node *rb_last(const rb_root_t *tree) 
{
    struct rb_node *node = tree->root;
    
    if (!node)
        return NULL;
    
    while (node->right)
        node = node->right;
    
    return node;
}

// 获取后继节点
struct rb_node *rb_next(const struct rb_node *node) 
{
    struct rb_node *current;
    
    if (!node)
        return NULL;
    
    // 如果有右子树，后继就是右子树中的最小值
    if (node->right) {
        current = node->right;
        while (current->left)
            current = current->left;
        return current;
    }
    
    // 否则，向上查找第一个"左拐"的祖先节点
    current = node->parent;
    while (current && current->right == node) {
        node = current;
        current = current->parent;
    }
    
    return current;
}

// 获取前驱节点
struct rb_node *rb_prev(const struct rb_node *node) 
{
    struct rb_node *current;
    
    if (!node)
        return NULL;
    
    // 如果有左子树，前驱就是左子树中的最大值
    if (node->left) {
        current = node->left;
        while (current->right)
            current = current->right;
        return current;
    }
    
    // 否则，向上查找第一个"右拐"的祖先节点
    current = node->parent;
    while (current && current->left == node) {
        node = current;
        current = current->parent;
    }
    
    return current;
}

// 判断红黑树是否为空
int rb_empty(const rb_root_t *tree) 
{
    return tree->root == NULL;
}

// 清空红黑树（不释放节点内存）
void rb_clear(rb_root_t *tree) 
{
    tree->root = NULL;
}

// 递归销毁红黑树节点
static void rb_destroy_recursive(rb_root_t *tree, struct rb_node *node) 
{
    if (!node)
        return;
    
    // 递归销毁子树
    rb_destroy_recursive(tree, node->left);
    rb_destroy_recursive(tree, node->right);
    
    // 如果提供了析构函数，调用它
    if (tree->node_destructor) {
        tree->node_destructor(node, tree->destructor_arg);
    }
}

// 销毁红黑树（递归释放所有节点内存）
void rb_destroy(rb_root_t *tree) 
{
    if (!tree)
        return;
    
    // 递归销毁所有节点
    rb_destroy_recursive(tree, tree->root);
    
    // 重置树根
    tree->root = NULL;
}

// 替换节点的颜色和指针
static void rb_transplant(rb_root_t *tree, struct rb_node *u, struct rb_node *v) 
{
    if (!u->parent) {
        tree->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    
    if (v)
        v->parent = u->parent;
}

// 删除修复
static void rb_erase_fixup(rb_root_t *tree, struct rb_node *node, struct rb_node *parent) 
{
    struct rb_node *sibling;
    
    while ((!node || node->color == RB_BLACK) && node != tree->root) {
        if (node == parent->left) {
            sibling = parent->right;
            
            // 情况1：兄弟节点是红色
            if (sibling->color == RB_RED) {
                sibling->color = RB_BLACK;
                parent->color = RB_RED;
                rb_rotate_left(tree, parent);
                sibling = parent->right;
            }
            
            // 情况2：兄弟节点是黑色，且两个子节点都是黑色
            if ((!sibling->left || sibling->left->color == RB_BLACK) &&
                (!sibling->right || sibling->right->color == RB_BLACK)) {
                sibling->color = RB_RED;
                node = parent;
                parent = node->parent;
            } else {
                // 情况3：兄弟节点是黑色，左子节点红色，右子节点黑色
                if (!sibling->right || sibling->right->color == RB_BLACK) {
                    if (sibling->left)
                        sibling->left->color = RB_BLACK;
                    sibling->color = RB_RED;
                    rb_rotate_right(tree, sibling);
                    sibling = parent->right;
                }
                
                // 情况4：兄弟节点是黑色，右子节点是红色
                sibling->color = parent->color;
                parent->color = RB_BLACK;
                if (sibling->right)
                    sibling->right->color = RB_BLACK;
                rb_rotate_left(tree, parent);
                node = tree->root;
                break;
            }
        } else {
            sibling = parent->left;
            
            // 情况1：兄弟节点是红色
            if (sibling->color == RB_RED) {
                sibling->color = RB_BLACK;
                parent->color = RB_RED;
                rb_rotate_right(tree, parent);
                sibling = parent->left;
            }
            
            // 情况2：兄弟节点是黑色，且两个子节点都是黑色
            if ((!sibling->right || sibling->right->color == RB_BLACK) &&
                (!sibling->left || sibling->left->color == RB_BLACK)) {
                sibling->color = RB_RED;
                node = parent;
                parent = node->parent;
            } else {
                // 情况3：兄弟节点是黑色，右子节点红色，左子节点黑色
                if (!sibling->left || sibling->left->color == RB_BLACK) {
                    if (sibling->right)
                        sibling->right->color = RB_BLACK;
                    sibling->color = RB_RED;
                    rb_rotate_left(tree, sibling);
                    sibling = parent->left;
                }
                
                // 情况4：兄弟节点是黑色，左子节点是红色
                sibling->color = parent->color;
                parent->color = RB_BLACK;
                if (sibling->left)
                    sibling->left->color = RB_BLACK;
                rb_rotate_right(tree, parent);
                node = tree->root;
                break;
            }
        }
    }
    
    if (node)
        node->color = RB_BLACK;
}

// 删除节点
void rb_erase(rb_root_t *tree, struct rb_node *node) 
{
    struct rb_node *child, *parent;
    enum rb_color color;
    
    // 保存需要调用析构函数的节点
    struct rb_node *to_destruct = node;
    
    // 情况1：左子节点为空
    if (!node->left) {
        child = node->right;
        parent = node->parent;
        color = node->color;
        
        rb_transplant(tree, node, child);
        
        // 如果删除的是黑色节点，需要修复
        if (color == RB_BLACK)
            rb_erase_fixup(tree, child, parent);
    }
    // 情况2：右子节点为空
    else if (!node->right) {
        child = node->left;
        parent = node->parent;
        color = node->color;
        
        rb_transplant(tree, node, child);
        
        // 如果删除的是黑色节点，需要修复
        if (color == RB_BLACK)
            rb_erase_fixup(tree, child, parent);
    }
    // 情况3：左右子节点都不为空
    else {
        // 找到后继节点
        struct rb_node *successor = node->right;
        while (successor->left)
            successor = successor->left;
        
        // 在这种情况下，后继节点将取代被删除的节点
        // 所以析构函数应该应用于被删除的节点而不是后继节点
        to_destruct = node;
        
        color = successor->color;
        child = successor->right;
        
        // 如果后继节点的父节点是当前节点
        if (successor->parent == node) {
            if (child)
                child->parent = successor;
            parent = successor;
        } else {
            parent = successor->parent;
            rb_transplant(tree, successor, successor->right);
            successor->right = node->right;
            successor->right->parent = successor;
        }
        
        rb_transplant(tree, node, successor);
        successor->left = node->left;
        successor->left->parent = successor;
        successor->color = node->color;
        
        // 如果后继节点是黑色，需要修复
        if (color == RB_BLACK)
            rb_erase_fixup(tree, child, parent);
    }
    
    // 清除被删除节点的指针
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    
    // 如果提供了析构函数，调用它
    if (tree->node_destructor) {
        tree->node_destructor(to_destruct, tree->destructor_arg);
    }
}

// 替换红黑树节点
void rb_replace(rb_root_t *tree, struct rb_node *old_node, struct rb_node *new_node) 
{
    // 复制节点关系和颜色
    new_node->left = old_node->left;
    new_node->right = old_node->right;
    new_node->parent = old_node->parent;
    new_node->color = old_node->color;
    
    // 更新子节点的父指针
    if (new_node->left)
        new_node->left->parent = new_node;
    if (new_node->right)
        new_node->right->parent = new_node;
    
    // 更新父节点的子指针
    if (old_node->parent) {
        if (old_node->parent->left == old_node)
            old_node->parent->left = new_node;
        else
            old_node->parent->right = new_node;
    } else {
        tree->root = new_node;
    }
    
    // 清除旧节点的指针
    old_node->left = NULL;
    old_node->right = NULL;
    old_node->parent = NULL;
    
    // 如果提供了析构函数，调用它
    if (tree->node_destructor) {
        tree->node_destructor(old_node, tree->destructor_arg);
    }
}

// 计算黑色节点高度（用于验证）
static int rb_black_height(const struct rb_node *node) 
{
    int height = 0;
    
    if (!node)
        return 1;
    
    // 如果当前节点是黑色，高度+1
    if (node->color == RB_BLACK)
        height = 1;
    
    // 计算左子树的黑色高度
    int left_height = rb_black_height(node->left);
    
    // 如果左子树有效，返回左子树高度+当前节点高度
    if (left_height > 0)
        return left_height + height;
    
    // 如果左子树无效，计算右子树
    int right_height = rb_black_height(node->right);
    if (right_height > 0)
        return right_height + height;
    
    // 如果左右子树都无效，返回无效值
    return -1;
}

// 验证红黑树的性质
static int rb_verify_node(const struct rb_node *node) 
{
    if (!node)
        return 1;
    
    // 性质1：节点是红色或黑色（通过枚举类型确保）
    
    // 性质4：红色节点的子节点必须是黑色
    if (node->color == RB_RED) {
        if ((node->left && node->left->color == RB_RED) ||
            (node->right && node->right->color == RB_RED))
            return 0;
    }
    
    // 递归验证子节点
    return rb_verify_node(node->left) && rb_verify_node(node->right);
}

// 验证红黑树合法性
int rb_verify(const rb_root_t *tree) 
{
    if (!tree->root)
        return 1;
    
    // 性质2：根节点是黑色
    if (tree->root->color != RB_BLACK)
        return 0;
    
    // 性质3和5：通过验证黑色高度一致来检查
    // 首先确保每个节点满足性质4
    if (!rb_verify_node(tree->root))
        return 0;
    
    // 验证所有路径的黑色节点数量相同
    int left_height = rb_black_height(tree->root->left);
    int right_height = rb_black_height(tree->root->right);
    
    return (left_height == right_height) && (left_height > 0);
}