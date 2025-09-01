#include "b_tree.h"
#include <string.h>

// 函数声明 - 应添加在文件开头
static int remove_key_recursive(struct btree_node *node, const void *key, int t, 
                              int (*compare)(const void *a, const void *b, void *arg),
                              void *compare_arg,
                              void (*key_destructor)(void *key, void *arg),
                              void *destructor_arg);

static void remove_from_internal(struct btree_node *node, int idx, int t, 
                               int (*compare)(const void *a, const void *b, void *arg),
                               void *compare_arg,
                               void (*key_destructor)(void *key, void *arg),
                               void *destructor_arg);

// 默认比较函数
static int default_compare(const void *a, const void *b, void *arg) {
    // 直接比较指针值
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

// 创建新的B树节点
static struct btree_node* create_node(int t, bool is_leaf) {
    struct btree_node *node = (struct btree_node*)malloc(sizeof(struct btree_node));
    if (!node) return NULL;
    
    node->n = 0;
    node->is_leaf = is_leaf;
    
    // 分配关键字数组空间，最多存储 2t-1 个关键字
    node->keys = (void**)malloc(sizeof(void*) * (2*t-1));
    if (!node->keys) {
        free(node);
        return NULL;
    }
    
    // 分配子节点指针数组空间，最多存储 2t 个子节点
    node->children = (struct btree_node**)malloc(sizeof(struct btree_node*) * (2*t));
    if (!node->children) {
        free(node->keys);
        free(node);
        return NULL;
    }
    
    // 初始化子节点指针为NULL
    for (int i = 0; i < 2*t; i++) {
        node->children[i] = NULL;
    }
    
    return node;
}

// 销毁B树节点
static void destroy_node(struct btree_node *node, 
                         void (*key_destructor)(void *key, void *arg),
                         void *destructor_arg) {
    if (!node) return;
    
    // 如果存在键析构函数，释放所有键
    if (key_destructor) {
        for (int i = 0; i < node->n; i++) {
            key_destructor(node->keys[i], destructor_arg);
        }
    }
    
    free(node->keys);
    free(node->children);
    free(node);
}

// 递归销毁B树
static void destroy_tree_recursive(struct btree_node *node, 
                                  void (*key_destructor)(void *key, void *arg),
                                  void *destructor_arg) {
    if (!node) return;
    
    // 如果不是叶节点，递归销毁所有子节点
    if (!node->is_leaf) {
        for (int i = 0; i <= node->n; i++) {
            destroy_tree_recursive(node->children[i], key_destructor, destructor_arg);
        }
    }
    
    // 销毁当前节点
    destroy_node(node, key_destructor, destructor_arg);
}

// 创建B树
btree_t* btree_create(int order, 
                     int (*compare)(const void *a, const void *b, void *arg),
                     void *compare_arg,
                     void (*key_destructor)(void *key, void *arg),
                     void *destructor_arg) {
    // 检查阶数，B树的阶必须至少为3
    if (order < 3) {
        order = BTREE_DEFAULT_ORDER;
    }
    
    btree_t *tree = (btree_t*)malloc(sizeof(btree_t));
    if (!tree) return NULL;
    
    tree->root = NULL;
    tree->order = order;
    tree->t = (order + 1) / 2;  // 计算最小度数
    tree->compare = compare ? compare : default_compare;
    tree->compare_arg = compare_arg;
    tree->key_destructor = key_destructor;
    tree->destructor_arg = destructor_arg;
    
    return tree;
}

// 销毁B树
void btree_destroy(btree_t *tree) {
    if (!tree) return;
    
    // 递归销毁所有节点
    destroy_tree_recursive(tree->root, tree->key_destructor, tree->destructor_arg);
    
    // 释放树结构
    free(tree);
}

// 查找节点中关键字的位置（二分查找）
static int find_key_index(const struct btree_node *node, const void *key, 
                         int (*compare)(const void *a, const void *b, void *arg),
                         void *compare_arg) {
    int left = 0;
    int right = node->n - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = compare(key, node->keys[mid], compare_arg);
        
        if (cmp == 0)
            return mid;  // 找到关键字
        else if (cmp < 0)
            right = mid - 1;
        else
            left = mid + 1;
    }
    
    return left;  // 未找到，返回应该插入的位置
}

// 在节点中搜索关键字，如果找到则返回位置，否则返回-1
static int search_key_in_node(const struct btree_node *node, const void *key,
                             int (*compare)(const void *a, const void *b, void *arg),
                             void *compare_arg) {
    int idx = find_key_index(node, key, compare, compare_arg);
    
    // 检查找到的位置是否有效且关键字匹配
    if (idx < node->n && compare(key, node->keys[idx], compare_arg) == 0)
        return idx;
    
    return -1;  // 未找到关键字
}

// 递归搜索B树
static void* search_recursive(const struct btree_node *node, const void *key,
                             int (*compare)(const void *a, const void *b, void *arg),
                             void *compare_arg) {
    if (!node) return NULL;
    
    // 在当前节点中查找关键字
    int i = search_key_in_node(node, key, compare, compare_arg);
    
    // 如果找到，返回关键字
    if (i != -1)
        return node->keys[i];
    
    // 如果是叶节点且未找到，返回NULL
    if (node->is_leaf)
        return NULL;
    
    // 确定应该在哪个子树中继续查找
    i = find_key_index(node, key, compare, compare_arg);
    
    // 递归在子树中查找
    return search_recursive(node->children[i], key, compare, compare_arg);
}

// 搜索关键字
void* btree_search(const btree_t *tree, const void *key) {
    if (!tree || !tree->root)
        return NULL;
    
    return search_recursive(tree->root, key, tree->compare, tree->compare_arg);
}

// 分裂子节点
static void split_child(struct btree_node *parent, int index, struct btree_node *child, int t) {
    // 创建新节点来存储分裂后的右半部分
    struct btree_node *new_node = create_node(t, child->is_leaf);
    if (!new_node) return;
    
    // 新节点将存储t-1个关键字
    new_node->n = t - 1;
    
    // 复制右半部分的关键字到新节点
    for (int j = 0; j < t-1; j++) {
        new_node->keys[j] = child->keys[j+t];
    }
    
    // 如果不是叶节点，也要复制右半部分的子节点指针
    if (!child->is_leaf) {
        for (int j = 0; j < t; j++) {
            new_node->children[j] = child->children[j+t];
        }
    }
    
    // 更新原子节点的关键字数量
    child->n = t - 1;
    
    // 将新节点插入到父节点的children数组中
    for (int j = parent->n; j >= index+1; j--) {
        parent->children[j+1] = parent->children[j];
    }
    parent->children[index+1] = new_node;
    
    // 将中间关键字移动到父节点
    for (int j = parent->n-1; j >= index; j--) {
        parent->keys[j+1] = parent->keys[j];
    }
    parent->keys[index] = child->keys[t-1];
    
    // 增加父节点的关键字数量
    parent->n++;
}

// 在非满节点中插入关键字
static void insert_non_full(struct btree_node *node, void *key, int t,
                           int (*compare)(const void *a, const void *b, void *arg),
                           void *compare_arg) {
    int i = node->n - 1;
    
    // 如果是叶节点，直接插入
    if (node->is_leaf) {
        // 找到正确的插入位置
        while (i >= 0 && compare(key, node->keys[i], compare_arg) < 0) {
            node->keys[i+1] = node->keys[i];
            i--;
        }
        
        node->keys[i+1] = key;
        node->n++;
    } else {
        // 找到要插入的子树
        i = find_key_index(node, key, compare, compare_arg);
        
        // 检查子节点是否已满
        if (node->children[i]->n == 2*t-1) {
            // 分裂子节点
            split_child(node, i, node->children[i], t);
            
            // 确定分裂后关键字应该插入到哪个子节点
            if (compare(key, node->keys[i], compare_arg) > 0)
                i++;
        }
        
        // 递归插入到子节点
        insert_non_full(node->children[i], key, t, compare, compare_arg);
    }
}

// 插入关键字
int btree_insert(btree_t *tree, void *key) {
    if (!tree) return -1;
    
    // 如果树为空，创建根节点
    if (!tree->root) {
        tree->root = create_node(tree->t, 1);
        if (!tree->root) return -1;
        
        tree->root->keys[0] = key;
        tree->root->n = 1;
        return 0;
    }
    
    // 检查是否已存在相同关键字
    void *found = btree_search(tree, key);
    if (found) return -1;  // 关键字已存在
    
    // 如果根节点已满，需要分裂
    if (tree->root->n == 2*tree->t-1) {
        struct btree_node *new_root = create_node(tree->t, 0);
        if (!new_root) return -1;
        
        new_root->children[0] = tree->root;
        tree->root = new_root;
        
        // 分裂原来的根节点
        split_child(new_root, 0, new_root->children[0], tree->t);
        
        // 在分裂后的树中插入关键字
        int i = 0;
        if (tree->compare(key, new_root->keys[0], tree->compare_arg) > 0)
            i++;
        
        insert_non_full(new_root->children[i], key, tree->t, 
                       tree->compare, tree->compare_arg);
    } else {
        // 直接在非满的根节点插入
        insert_non_full(tree->root, key, tree->t, tree->compare, tree->compare_arg);
    }
    
    return 0;
}

// 查找最小关键字
static void* find_min_key(const struct btree_node *node) {
    // 如果节点为空，返回NULL
    if (!node) return NULL;
    
    // 一直向左子树寻找，直到叶节点
    while (!node->is_leaf) {
        node = node->children[0];
    }
    
    // 返回最左叶节点的第一个关键字
    return node->n > 0 ? node->keys[0] : NULL;
}

// 查找最大关键字
static void* find_max_key(const struct btree_node *node) {
    // 如果节点为空，返回NULL
    if (!node) return NULL;
    
    // 一直向右子树寻找，直到叶节点
    while (!node->is_leaf) {
        node = node->children[node->n];
    }
    
    // 返回最右叶节点的最后一个关键字
    return node->n > 0 ? node->keys[node->n-1] : NULL;
}

// 获取B树中最小关键字
void* btree_get_min(const btree_t *tree) {
    if (!tree) return NULL;
    return find_min_key(tree->root);
}

// 获取B树中最大关键字
void* btree_get_max(const btree_t *tree) {
    if (!tree) return NULL;
    return find_max_key(tree->root);
}

// 合并节点 - 修改合并节点函数 - 避免释放被移动的关键字
static void merge_nodes(struct btree_node *parent, int idx, int t) {
    struct btree_node *child = parent->children[idx];
    struct btree_node *sibling = parent->children[idx+1];
    
    // 将父节点中的关键字移动到子节点的中间位置
    child->keys[t-1] = parent->keys[idx];
    
    // 将右兄弟节点的关键字复制到子节点
    for (int i = 0; i < sibling->n; i++) {
        child->keys[i+t] = sibling->keys[i];
    }
    
    // 如果不是叶节点，复制右兄弟的子节点指针
    if (!child->is_leaf) {
        for (int i = 0; i <= sibling->n; i++) {
            child->children[i+t] = sibling->children[i];
        }
    }
    
    // 更新子节点关键字数量
    child->n = 2*t - 1;
    
    // 从父节点中移除关键字和右兄弟节点的指针
    for (int i = idx; i < parent->n-1; i++) {
        parent->keys[i] = parent->keys[i+1];
        parent->children[i+1] = parent->children[i+2];
    }
    
    // 减少父节点关键字数量
    parent->n--;
    
    // 释放右兄弟节点（但不释放它的键，因为键已被移动）
    free(sibling->keys);
    free(sibling->children);
    free(sibling);
}

// 从子节点借一个关键字
static void borrow_from_prev(struct btree_node *node, int idx, int t) {
    struct btree_node *child = node->children[idx];
    struct btree_node *sibling = node->children[idx-1];
    
    // 为要借入的关键字腾出空间
    for (int i = child->n-1; i >= 0; i--) {
        child->keys[i+1] = child->keys[i];
    }
    
    // 如果不是叶节点，也要移动子节点指针
    if (!child->is_leaf) {
        for (int i = child->n; i >= 0; i--) {
            child->children[i+1] = child->children[i];
        }
    }
    
    // 将父节点中的关键字下移到子节点的首位
    child->keys[0] = node->keys[idx-1];
    
    // 如果不是叶节点，将左兄弟的最右子节点转移给子节点
    if (!child->is_leaf) {
        child->children[0] = sibling->children[sibling->n];
    }
    
    // 将左兄弟的最后一个关键字上移到父节点
    node->keys[idx-1] = sibling->keys[sibling->n-1];
    
    // 更新关键字数量
    child->n++;
    sibling->n--;
}

// 从右兄弟借一个关键字
static void borrow_from_next(struct btree_node *node, int idx, int t) {
    struct btree_node *child = node->children[idx];
    struct btree_node *sibling = node->children[idx+1];
    
    // 将父节点中的关键字下移到子节点的末尾
    child->keys[child->n] = node->keys[idx];
    
    // 如果不是叶节点，将右兄弟的最左子节点转移给子节点
    if (!child->is_leaf) {
        child->children[child->n+1] = sibling->children[0];
    }
    
    // 将右兄弟的第一个关键字上移到父节点
    node->keys[idx] = sibling->keys[0];
    
    // 在右兄弟中移除已借出的关键字和子节点
    for (int i = 0; i < sibling->n-1; i++) {
        sibling->keys[i] = sibling->keys[i+1];
    }
    
    if (!sibling->is_leaf) {
        for (int i = 0; i < sibling->n; i++) {
            sibling->children[i] = sibling->children[i+1];
        }
    }
    
    // 更新关键字数量
    child->n++;
    sibling->n--;
}

// 确保子节点至少有t个关键字
static void fill_child(struct btree_node *node, int idx, int t) {
    // 如果前一个兄弟有多余的关键字，借用
    if (idx > 0 && node->children[idx-1]->n >= t) {
        borrow_from_prev(node, idx, t);
    }
    // 如果后一个兄弟有多余的关键字，借用
    else if (idx < node->n && node->children[idx+1]->n >= t) {
        borrow_from_next(node, idx, t);
    }
    // 如果两个兄弟都没有多余关键字，合并节点
    else {
        if (idx < node->n) {
            merge_nodes(node, idx, t);
        } else {
            merge_nodes(node, idx-1, t);
        }
    }
}

// 从内部节点中删除关键字 - 修正内存管理问题
static void remove_from_internal(struct btree_node *node, int idx, int t, 
                               int (*compare)(const void *a, const void *b, void *arg),
                               void *compare_arg,
                               void (*key_destructor)(void *key, void *arg),
                               void *destructor_arg) {
    void *key = node->keys[idx];
    
    // 情况1：如果左子树至少有t个关键字，找前驱替换，并递归删除前驱
    if (node->children[idx]->n >= t) {
        // 找到前驱
        struct btree_node *curr = node->children[idx];
        while (!curr->is_leaf) {
            curr = curr->children[curr->n];
        }
        void *pred = curr->keys[curr->n-1];
        
        // 用前驱替换当前关键字
        node->keys[idx] = pred;
        
        // 递归删除前驱（不释放键，因为键已被移动）
        remove_key_recursive(node->children[idx], pred, t, 
                           compare, compare_arg, 
                           NULL, destructor_arg);  // 传NULL防止重复释放
    }
    // 情况2：如果右子树至少有t个关键字，找后继替换，并递归删除后继
    else if (node->children[idx+1]->n >= t) {
        // 找到后继
        struct btree_node *curr = node->children[idx+1];
        while (!curr->is_leaf) {
            curr = curr->children[0];
        }
        void *succ = curr->keys[0];
        
        // 用后继替换当前关键字
        node->keys[idx] = succ;
        
        // 递归删除后继（不释放键，因为键已被移动）
        remove_key_recursive(node->children[idx+1], succ, t, 
                           compare, compare_arg,
                           NULL, destructor_arg);  // 传NULL防止重复释放
    }
    // 情况3：如果左右子树都少于t个关键字，合并子节点，递归删除
    else {
        merge_nodes(node, idx, t);
        
        // 在合并后的节点中查找并删除关键字
        remove_key_recursive(node->children[idx], key, t, 
                           compare, compare_arg,
                           key_destructor, destructor_arg);
        
        // 由于递归删除已经释放了关键字，这里不再释放
        return;
    }
    
    // 释放被替换的原始关键字
    if (key_destructor) {
        key_destructor(key, destructor_arg);
    }
}

// 从叶节点中删除关键字 - 保留原方法不变
static void remove_from_leaf(struct btree_node *node, int idx, 
                           void (*key_destructor)(void *key, void *arg),
                           void *destructor_arg) {
    void *key = node->keys[idx];
    
    // 移动后面的关键字向前
    for (int i = idx; i < node->n-1; i++) {
        node->keys[i] = node->keys[i+1];
    }
    
    // 减少节点关键字数量
    node->n--;
    
    // 如果需要，销毁被删除的关键字
    if (key_destructor) {
        key_destructor(key, destructor_arg);
    }
}

// 递归删除关键字 - 确保正确的内存管理
static int remove_key_recursive(struct btree_node *node, const void *key, int t, 
                              int (*compare)(const void *a, const void *b, void *arg),
                              void *compare_arg,
                              void (*key_destructor)(void *key, void *arg),
                              void *destructor_arg) {
    // 如果节点为空，返回未找到
    if (!node) return -1;
    
    // 在当前节点查找关键字
    int idx = search_key_in_node(node, key, compare, compare_arg);
    
    // 如果找到关键字，并且在当前节点
    if (idx != -1) {
        // 如果是叶节点，直接删除
        if (node->is_leaf) {
            remove_from_leaf(node, idx, key_destructor, destructor_arg);
        } else {
            // 从内部节点删除关键字
            remove_from_internal(node, idx, t, compare, compare_arg, key_destructor, destructor_arg);
        }
        return 0;
    }
    
    // 如果是叶节点但未找到关键字，返回未找到
    if (node->is_leaf) {
        return -1;
    }
    
    // 确定应该在哪个子树中继续查找
    idx = find_key_index(node, key, compare, compare_arg);
    bool last_child = (idx == node->n);
    
    // 确保子节点至少有t个关键字
    if (node->children[idx]->n < t) {
        fill_child(node, idx, t);
    }
    
    // 当子节点被合并，调整idx
    if (last_child && idx > node->n) {
        return remove_key_recursive(node->children[idx-1], key, t, 
                                  compare, compare_arg,
                                  key_destructor, destructor_arg);
    } else {
        return remove_key_recursive(node->children[idx], key, t, 
                                  compare, compare_arg,
                                  key_destructor, destructor_arg);
    }
}

// 删除关键字 - 修改函数实现，传递比较函数
int btree_delete(btree_t *tree, const void *key) {
    if (!tree || !tree->root) return -1;
    
    int result = remove_key_recursive(tree->root, key, tree->t, 
                                   tree->compare, tree->compare_arg,
                                   tree->key_destructor, tree->destructor_arg);
    
    // 如果根节点只有一个子节点，更新根
    if (tree->root->n == 0 && !tree->root->is_leaf) {
        struct btree_node *old_root = tree->root;
        tree->root = tree->root->children[0];
        
        // 释放旧的根节点
        free(old_root->keys);
        free(old_root->children);
        free(old_root);
    }
    
    return result;
}

// 递归计算B树的高度
static int height_recursive(const struct btree_node *node) {
    if (!node) return 0;
    
    // 如果是叶节点，高度为1
    if (node->is_leaf) return 1;
    
    // 递归计算子节点的高度，取最大值
    return 1 + height_recursive(node->children[0]);
}

// 获取B树的高度
int btree_height(const btree_t *tree) {
    if (!tree || !tree->root) return 0;
    return height_recursive(tree->root);
}

// 递归计算B树关键字数量
static int count_keys_recursive(const struct btree_node *node) {
    if (!node) return 0;
    
    int count = node->n;
    
    // 如果不是叶节点，递归计算所有子节点的关键字
    if (!node->is_leaf) {
        for (int i = 0; i <= node->n; i++) {
            count += count_keys_recursive(node->children[i]);
        }
    }
    
    return count;
}

// 获取B树中关键字数量
int btree_count(const btree_t *tree) {
    if (!tree) return 0;
    return count_keys_recursive(tree->root);
}

// 清空B树
void btree_clear(btree_t *tree) {
    if (!tree) return;
    
    // 递归销毁所有节点
    destroy_tree_recursive(tree->root, tree->key_destructor, tree->destructor_arg);
    
    // 重置根节点
    tree->root = NULL;
}

// 检查B树是否为空
bool btree_empty(const btree_t *tree) {
    return !tree || !tree->root || tree->root->n == 0;
}

// 递归中序遍历
static void inorder_recursive(const struct btree_node *node, 
                             void (*callback)(void *key, void *arg),
                             void *arg) {
    if (!node) return;
    
    // 如果是叶节点，直接遍历所有关键字
    if (node->is_leaf) {
        for (int i = 0; i < node->n; i++) {
            callback(node->keys[i], arg);
        }
        return;
    }
    
    // 遍历内部节点：左子树 -> 关键字 -> 右子树
    for (int i = 0; i < node->n; i++) {
        // 遍历左子树
        inorder_recursive(node->children[i], callback, arg);
        
        // 访问当前关键字
        callback(node->keys[i], arg);
    }
    
    // 遍历最后一个右子树
    inorder_recursive(node->children[node->n], callback, arg);
}

// 中序遍历B树
void btree_inorder(const btree_t *tree,
                  void (*callback)(void *key, void *arg),
                  void *arg) {
    if (!tree || !tree->root || !callback) return;
    inorder_recursive(tree->root, callback, arg);
}

// 递归遍历B树（层序遍历）
static void foreach_recursive(const struct btree_node *node, 
                            void (*callback)(void *key, void *arg),
                            void *arg) {
    if (!node) return;
    
    // 先访问当前节点的所有关键字
    for (int i = 0; i < node->n; i++) {
        callback(node->keys[i], arg);
    }
    
    // 如果不是叶节点，递归访问所有子节点
    if (!node->is_leaf) {
        for (int i = 0; i <= node->n; i++) {
            foreach_recursive(node->children[i], callback, arg);
        }
    }
}

// 遍历B树
void btree_foreach(const btree_t *tree, 
                  void (*callback)(void *key, void *arg),
                  void *arg) {
    if (!tree || !tree->root || !callback) return;
    foreach_recursive(tree->root, callback, arg);
}