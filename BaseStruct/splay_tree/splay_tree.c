#include "splay_tree.h"

// 初始化伸展树
void splay_init(splay_root_t *tree, 
                int (*compare)(const struct splay_node *a, 
                               const struct splay_node *b, void *arg),
                void *compare_arg,
                void (*node_destructor)(struct splay_node *node, void *arg),
                void *destructor_arg) 
{
    tree->root = NULL;
    tree->compare = compare;
    tree->compare_arg = compare_arg;
    tree->node_destructor = node_destructor;
    tree->destructor_arg = destructor_arg;
}

// 将节点旋转到其父节点位置
static void splay_rotate(splay_root_t *tree, struct splay_node *node) 
{
    struct splay_node *parent = node->parent;
    struct splay_node *grandparent = parent->parent;
    
    if (parent->left == node) {
        // 左子节点情况
        parent->left = node->right;
        if (node->right)
            node->right->parent = parent;
        node->right = parent;
    } else {
        // 右子节点情况
        parent->right = node->left;
        if (node->left)
            node->left->parent = parent;
        node->left = parent;
    }
    
    // 更新父节点关系
    node->parent = grandparent;
    parent->parent = node;
    
    // 更新祖父节点指针
    if (grandparent) {
        if (grandparent->left == parent)
            grandparent->left = node;
        else
            grandparent->right = node;
    } else {
        // 如果没有祖父节点，说明node现在应该是根节点
        tree->root = node;
    }
}

// 伸展操作 - 将节点旋转到根
static void splay(splay_root_t *tree, struct splay_node *node) 
{
    if (!node) return;
    
    while (node->parent) {
        struct splay_node *parent = node->parent;
        struct splay_node *grandparent = parent->parent;
        
        if (!grandparent) {
            // Zig步骤：节点只有一个父节点
            splay_rotate(tree, node);
        } else {
            // Zig-Zig或Zig-Zag步骤
            int is_left_child = (parent->left == node);
            int is_parent_left_child = (grandparent->left == parent);
            
            if (is_left_child == is_parent_left_child) {
                // Zig-Zig：先旋转父节点，再旋转当前节点
                splay_rotate(tree, parent);
                splay_rotate(tree, node);
            } else {
                // Zig-Zag：旋转当前节点两次
                splay_rotate(tree, node);
                splay_rotate(tree, node);
            }
        }
    }
}

// 查找节点并伸展
struct splay_node *splay_search(splay_root_t *tree, struct splay_node *key) 
{
    struct splay_node *current = tree->root;
    struct splay_node *last_accessed = NULL;
    int cmp;
    
    if (!current) return NULL;
    
    while (current) {
        last_accessed = current;
        cmp = tree->compare(key, current, tree->compare_arg);
        
        if (cmp < 0)
            current = current->left;
        else if (cmp > 0)
            current = current->right;
        else {
            // 找到节点，将其伸展到根部
            splay(tree, current);
            return current;
        }
    }
    
    // 没找到节点，将最后访问的节点伸展到根部
    if (last_accessed)
        splay(tree, last_accessed);
    
    return NULL;
}

// 插入节点
int splay_insert(splay_root_t *tree, struct splay_node *node) 
{
    struct splay_node *current = tree->root;
    struct splay_node *parent = NULL;
    int cmp;
    
    // 初始化新节点
    node->left = NULL;
    node->right = NULL;
    
    // 处理空树情况
    if (!current) {
        node->parent = NULL;
        tree->root = node;
        return 0;
    }
    
    // 搜索插入位置
    while (current) {
        parent = current;
        cmp = tree->compare(node, current, tree->compare_arg);
        
        if (cmp < 0)
            current = current->left;
        else if (cmp > 0)
            current = current->right;
        else {
            // 节点已存在，伸展存在的节点
            splay(tree, current);
            return -1; // 插入失败，节点已存在
        }
    }
    
    // 插入新节点
    node->parent = parent;
    if (cmp < 0)
        parent->left = node;
    else
        parent->right = node;
    
    // 将新节点伸展到根部
    splay(tree, node);
    return 0;
}

// 查找最小值节点
struct splay_node *splay_first(const splay_root_t *tree) 
{
    struct splay_node *node = tree->root;
    
    if (!node)
        return NULL;
        
    while (node->left)
        node = node->left;
        
    return node;
}

// 查找最大值节点
struct splay_node *splay_last(const splay_root_t *tree) 
{
    struct splay_node *node = tree->root;
    
    if (!node)
        return NULL;
        
    while (node->right)
        node = node->right;
        
    return node;
}

// 查找后继节点
struct splay_node *splay_next(const struct splay_node *node) 
{
    struct splay_node *current;
    
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

// 查找前驱节点
struct splay_node *splay_prev(const struct splay_node *node) 
{
    struct splay_node *current;
    
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
void splay_erase(splay_root_t *tree, struct splay_node *node) 
{
    struct splay_node *replacement = NULL;
    
    // 先将节点伸展到根部
    splay(tree, node);
    
    if (!node->left) {
        // 没有左子树，直接用右子树替换
        replacement = node->right;
        if (replacement)
            replacement->parent = NULL;
    } else {
        // 有左子树，找到左子树的最大值作为替代节点
        replacement = node->left;
        
        // 将右子树连接到左子树的最右节点
        if (node->right) {
            while (replacement->right)
                replacement = replacement->right;
                
            replacement->right = node->right;
            node->right->parent = replacement;
        }
        
        // 断开左子树与被删除节点的连接
        node->left->parent = NULL;
        replacement = node->left;
    }
    
    // 更新根节点
    tree->root = replacement;
    
    // 清除被删除节点的指针
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    
    // 如果提供了析构函数，调用它
    if (tree->node_destructor) {
        tree->node_destructor(node, tree->destructor_arg);
    }
}

// 判断伸展树是否为空
int splay_empty(const splay_root_t *tree) 
{
    return tree->root == NULL;
}

// 清空伸展树（不释放节点内存）
void splay_clear(splay_root_t *tree) 
{
    tree->root = NULL;
}

// 递归销毁伸展树节点
static void destroy_tree_recursive(splay_root_t *tree, struct splay_node *node) 
{
    if (!node)
        return;
    
    // 递归销毁子树
    destroy_tree_recursive(tree, node->left);
    destroy_tree_recursive(tree, node->right);
    
    // 如果提供了析构函数，调用它
    if (tree->node_destructor) {
        tree->node_destructor(node, tree->destructor_arg);
    }
}

// 销毁伸展树（递归释放所有节点内存）
void splay_destroy(splay_root_t *tree) 
{
    if (!tree)
        return;
    
    // 递归销毁所有节点
    destroy_tree_recursive(tree, tree->root);
    
    // 重置树根
    tree->root = NULL;
}

// 替换伸展树节点
void splay_replace(splay_root_t *tree, 
                  struct splay_node *old_node, 
                  struct splay_node *new_node) 
{
    // 复制节点关系
    new_node->left = old_node->left;
    new_node->right = old_node->right;
    new_node->parent = old_node->parent;
    
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
    
    // 清除旧节点的指针
    old_node->left = NULL;
    old_node->right = NULL;
    old_node->parent = NULL;
    
    // 如果提供了析构函数，调用它
    if (tree->node_destructor) {
        tree->node_destructor(old_node, tree->destructor_arg);
    }
}