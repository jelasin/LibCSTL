#include "avl_tree.h"

// 初始化AVL树
void avl_init(avl_root_t *tree, 
              int (*compare)(const struct avl_node *a, 
                            const struct avl_node *b, void *arg),
              void *compare_arg,
              void (*node_destructor)(struct avl_node *node, void *arg),
              void *destructor_arg) 
{
    tree->root = NULL;
    tree->compare = compare;
    tree->compare_arg = compare_arg;
    tree->node_destructor = node_destructor;
    tree->destructor_arg = destructor_arg;
}

// 获取节点高度
int avl_height(const struct avl_node *node) 
{
    return node ? node->height : 0;
}

// 获取节点平衡因子 (左子树高度 - 右子树高度)
int avl_balance_factor(const struct avl_node *node) 
{
    return node ? avl_height(node->left) - avl_height(node->right) : 0;
}

// 更新节点高度
static void update_height(struct avl_node *node) 
{
    int left_height = avl_height(node->left);
    int right_height = avl_height(node->right);
    node->height = (left_height > right_height ? left_height : right_height) + 1;
}

// 右旋转
static struct avl_node *rotate_right(avl_root_t *tree, struct avl_node *y) 
{
    struct avl_node *x = y->left;
    struct avl_node *T2 = x->right;
    struct avl_node *parent = y->parent;
    
    // 执行旋转
    x->right = y;
    y->left = T2;
    
    // 更新父节点指针
    x->parent = parent;
    y->parent = x;
    if (T2) T2->parent = y;
    
    // 更新父节点的子节点指针
    if (parent) {
        if (parent->left == y)
            parent->left = x;
        else
            parent->right = x;
    } else {
        tree->root = x;
    }
    
    // 更新高度
    update_height(y);
    update_height(x);
    
    return x;
}

// 左旋转
static struct avl_node *rotate_left(avl_root_t *tree, struct avl_node *x) 
{
    struct avl_node *y = x->right;
    struct avl_node *T2 = y->left;
    struct avl_node *parent = x->parent;
    
    // 执行旋转
    y->left = x;
    x->right = T2;
    
    // 更新父节点指针
    y->parent = parent;
    x->parent = y;
    if (T2) T2->parent = x;
    
    // 更新父节点的子节点指针
    if (parent) {
        if (parent->left == x)
            parent->left = y;
        else
            parent->right = y;
    } else {
        tree->root = y;
    }
    
    // 更新高度
    update_height(x);
    update_height(y);
    
    return y;
}

// 对节点进行平衡操作
static struct avl_node *balance_node(avl_root_t *tree, struct avl_node *node) 
{
    // 更新节点高度
    update_height(node);
    
    // 获取平衡因子
    int balance = avl_balance_factor(node);
    
    // 左左情况 - 右旋
    if (balance > 1 && avl_balance_factor(node->left) >= 0)
        return rotate_right(tree, node);
    
    // 左右情况 - 先左旋后右旋
    if (balance > 1 && avl_balance_factor(node->left) < 0) {
        node->left = rotate_left(tree, node->left);
        return rotate_right(tree, node);
    }
    
    // 右右情况 - 左旋
    if (balance < -1 && avl_balance_factor(node->right) <= 0)
        return rotate_left(tree, node);
    
    // 右左情况 - 先右旋后左旋
    if (balance < -1 && avl_balance_factor(node->right) > 0) {
        node->right = rotate_right(tree, node->right);
        return rotate_left(tree, node);
    }
    
    // 节点已经平衡
    return node;
}

// 查找节点
struct avl_node *avl_search(const avl_root_t *tree, const struct avl_node *key) 
{
    struct avl_node *current = tree->root;
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

// 插入节点
int avl_insert(avl_root_t *tree, struct avl_node *node) 
{
    // 初始化新节点
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    
    // 处理空树情况
    if (!tree->root) {
        node->parent = NULL;
        tree->root = node;
        return 0;
    }
    
    struct avl_node *current = tree->root;
    struct avl_node *parent = NULL;
    int cmp;
    
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
    
    // 从插入点向上回溯，重新平衡树
    current = node;
    while (parent) {
        update_height(parent);
        
        // 检查是否需要重新平衡
        int balance = avl_balance_factor(parent);
        if (balance > 1 || balance < -1) {
            balance_node(tree, parent);
        }
        
        current = parent;
        parent = parent->parent;
    }
    
    return 0;
}

// 获取最小值节点
struct avl_node *avl_first(const avl_root_t *tree) 
{
    struct avl_node *node = tree->root;
    
    if (!node)
        return NULL;
    
    while (node->left)
        node = node->left;
    
    return node;
}

// 获取最大值节点
struct avl_node *avl_last(const avl_root_t *tree) 
{
    struct avl_node *node = tree->root;
    
    if (!node)
        return NULL;
    
    while (node->right)
        node = node->right;
    
    return node;
}

// 获取后继节点
struct avl_node *avl_next(const struct avl_node *node) 
{
    struct avl_node *current;
    
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
struct avl_node *avl_prev(const struct avl_node *node) 
{
    struct avl_node *current;
    
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

// 删除节点
void avl_erase(avl_root_t *tree, struct avl_node *node) 
{
    // 保存需要平衡的起点；侵入式语义：不在此处析构用户节点
    struct avl_node *balance_start = node->parent;
    
    // 处理叶子节点的情况
    if (!node->left && !node->right) {
        if (!node->parent) {
            tree->root = NULL;
        } else {
            if (node->parent->left == node)
                node->parent->left = NULL;
            else
                node->parent->right = NULL;
        }
    }
    // 只有左子树的情况
    else if (node->left && !node->right) {
        if (!node->parent) {
            tree->root = node->left;
            node->left->parent = NULL;
        } else {
            node->left->parent = node->parent;
            if (node->parent->left == node)
                node->parent->left = node->left;
            else
                node->parent->right = node->left;
        }
    }
    // 只有右子树的情况
    else if (!node->left && node->right) {
        if (!node->parent) {
            tree->root = node->right;
            node->right->parent = NULL;
        } else {
            node->right->parent = node->parent;
            if (node->parent->left == node)
                node->parent->left = node->right;
            else
                node->parent->right = node->right;
        }
    }
    // 左右子树都存在的情况
    else {
        // 找到后继节点（右子树中的最小值）
        struct avl_node *successor = node->right;
        while (successor->left)
            successor = successor->left;
        
        // 如果后继节点不是右子节点，需要特殊处理
        if (successor->parent != node) {
            balance_start = successor->parent;
            
            // 将后继节点的右子树连接到后继节点的父节点上
            if (successor->right) {
                successor->right->parent = successor->parent;
                successor->parent->left = successor->right;
            } else {
                successor->parent->left = NULL;
            }
            
            // 重建后继节点与节点的右子树的关系
            successor->right = node->right;
            node->right->parent = successor;
        } else {
            balance_start = successor;
        }
        
        // 更新后继节点与节点的父节点和左子树的关系
        if (!node->parent) {
            tree->root = successor;
            successor->parent = NULL;
        } else {
            successor->parent = node->parent;
            if (node->parent->left == node)
                node->parent->left = successor;
            else
                node->parent->right = successor;
        }
        
        successor->left = node->left;
        node->left->parent = successor;
    }
    
    // 清除被删除节点的指针
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    
    // 从删除点向上回溯，重新平衡树
    while (balance_start) {
        update_height(balance_start);
        
        // 检查是否需要重新平衡
        int balance = avl_balance_factor(balance_start);
        if (balance > 1 || balance < -1) {
            balance_start = balance_node(tree, balance_start);
        }
        
        balance_start = balance_start->parent;
    }
    
    // 侵入式语义：不在单点删除中调用析构，由调用者自行释放；
    // 统一在 avl_destroy 中集中析构
}

// 判断AVL树是否为空
int avl_empty(const avl_root_t *tree) 
{
    return tree->root == NULL;
}

// 递归销毁AVL树节点
static void destroy_tree_recursive(avl_root_t *tree, struct avl_node *node) 
{
    if (!node)
        return;
    
    // 递归销毁子树
    destroy_tree_recursive(tree, node->left);
    destroy_tree_recursive(tree, node->right);
    
    // 由库负责断开节点与树的关联，再交给用户析构资源
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    // 如果提供了析构函数，调用它
    if (tree->node_destructor) {
        tree->node_destructor(node, tree->destructor_arg);
    }
}

// 销毁AVL树（递归释放所有节点内存）
void avl_destroy(avl_root_t *tree) 
{
    if (!tree)
        return;
    
    // 递归销毁所有节点
    destroy_tree_recursive(tree, tree->root);
    
    // 重置树根
    tree->root = NULL;
}

// 替换AVL树节点
void avl_replace(avl_root_t *tree, 
               struct avl_node *old_node, 
               struct avl_node *new_node) 
{
    // 复制节点关系和高度
    new_node->left = old_node->left;
    new_node->right = old_node->right;
    new_node->parent = old_node->parent;
    new_node->height = old_node->height;
    
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
        // 如果是根节点，更新根指针
        tree->root = new_node;
    }
    
    // 清除旧节点的指针（不在此处析构，保持侵入式语义）
    old_node->left = NULL;
    old_node->right = NULL;
    old_node->parent = NULL;
    old_node->height = 1;
}