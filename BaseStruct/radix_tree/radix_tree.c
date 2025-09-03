#include "radix_tree.h"

#include <string.h>
#include <stdio.h>

// 内部节点结构（混合模式：支持压缩路径和固定分支）
struct radix_node {
	// 原有压缩前缀树模式字段
	struct radix_node *parent;
	struct radix_node *first_child;   // 子链表头（用于压缩前缀模式）
	struct radix_node *next_sibling;  // 兄弟链（用于压缩前缀模式）

	unsigned char *label;             // 从父到此的字节标签（可为空）
	size_t label_len;                 // 标签长度

	struct radix_leaf *leaf;          // 若此节点为终止节点，指向叶子

	// 新增：Linux内核风格固定分支模式字段
	void **slots;                     // 固定大小的子节点/项目数组
	unsigned long tags[RADIX_TREE_MAX_TAGS]; // 标记位图
	unsigned char shift;              // 该节点在树中的层级（位移值）
	unsigned char count;              // 非空槽位数量
	unsigned char is_fixed_mode;      // 1=固定分支模式，0=压缩前缀模式
	unsigned char exceptional;        // 异常条目标志（保留）
};

static struct radix_node *radix_node_new(const unsigned char *label, size_t len)
{
	struct radix_node *n = (struct radix_node *)malloc(sizeof(*n));
	if (!n) return NULL;
	
	// 初始化通用字段
	n->parent = NULL;
	n->first_child = NULL;
	n->next_sibling = NULL;
	n->leaf = NULL;
	n->label = NULL;
	n->label_len = 0;
	
	// 初始化固定分支模式字段
	n->slots = NULL;
	memset(n->tags, 0, sizeof(n->tags));
	n->shift = 0;
	n->count = 0;
	n->is_fixed_mode = 0;  // 默认压缩前缀模式
	n->exceptional = 0;
	
	// 设置标签（用于压缩前缀模式）
	if (len) {
		n->label = (unsigned char *)malloc(len);
		if (!n->label) {
			free(n);
			return NULL;
		}
		memcpy(n->label, label, len);
		n->label_len = len;
	}
	return n;
}

// 为固定分支模式分配节点
static struct radix_node *radix_node_new_fixed(unsigned char shift)
{
	struct radix_node *n = (struct radix_node *)malloc(sizeof(*n));
	if (!n) return NULL;
	
	// 分配固定大小的槽位数组
	n->slots = (void **)calloc(RADIX_TREE_MAP_SIZE, sizeof(void*));
	if (!n->slots) {
		free(n);
		return NULL;
	}
	
	// 初始化字段
	n->parent = NULL;
	n->first_child = NULL;
	n->next_sibling = NULL;
	n->leaf = NULL;
	n->label = NULL;
	n->label_len = 0;
	
	memset(n->tags, 0, sizeof(n->tags));
	n->shift = shift;
	n->count = 0;
	n->is_fixed_mode = 1;  // 固定分支模式
	n->exceptional = 0;
	
	return n;
}

static void radix_node_free(struct radix_node *n)
{
	if (!n) return;
	if (n->label) free(n->label);
	if (n->slots) free(n->slots);
	free(n);
}

// 将 child 插入 parent 的子链首
static void add_child(struct radix_node *parent, struct radix_node *child)
{
	child->parent = parent;
	child->next_sibling = parent->first_child;
	parent->first_child = child;
}

// 在 parent 的子链中按首字节查找匹配的子
static struct radix_node *find_child_by_first(const struct radix_node *parent, unsigned char c)
{
	for (struct radix_node *ch = parent->first_child; ch; ch = ch->next_sibling) {
		if (ch->label_len > 0 && ch->label[0] == c) return ch;
	}
	return NULL;
}

// ============================================================================
// Linux内核风格辅助函数
// ============================================================================

// 计算键在指定层级的索引
static inline unsigned long radix_tree_index(unsigned long index, unsigned int shift)
{
	return (index >> shift) & RADIX_TREE_MAP_MASK;
}

// 检查是否为内部节点
static inline int radix_tree_is_internal_node(void *ptr)
{
	return ((unsigned long)ptr & 3) == 2;
}

// 从内部节点指针中提取实际节点指针
static inline struct radix_node *entry_to_node(void *ptr)
{
	return (struct radix_node *)((unsigned long)ptr & ~3UL);
}

// 将节点指针转换为内部节点标记指针
static inline void *node_to_entry(struct radix_node *ptr)
{
	return (void *)((unsigned long)ptr | 2);
}

// 标记操作辅助函数
static inline void tag_set(struct radix_node *node, unsigned int tag, int offset)
{
	if (tag < RADIX_TREE_MAX_TAGS)
		node->tags[tag] |= 1UL << offset;
}

static inline void tag_clear(struct radix_node *node, unsigned int tag, int offset)
{
	if (tag < RADIX_TREE_MAX_TAGS)
		node->tags[tag] &= ~(1UL << offset);
}

static inline int tag_get(struct radix_node *node, unsigned int tag, int offset)
{
	if (tag >= RADIX_TREE_MAX_TAGS)
		return 0;
	return !!(node->tags[tag] & (1UL << offset));
}

static inline int any_tag_set(struct radix_node *node, unsigned int tag)
{
	if (tag >= RADIX_TREE_MAX_TAGS)
		return 0;
	return node->tags[tag] != 0;
}

// 计算与标签的最长公共前缀
static size_t common_prefix(const unsigned char *a, size_t la, const unsigned char *b, size_t lb)
{
	size_t i = 0, m = la < lb ? la : lb;
	while (i < m && a[i] == b[i]) ++i;
	return i;
}

// 标签的字典序比较：<0 a<b, 0 相等, >0 a>b
static int cmp_label(const unsigned char *a, size_t la, const unsigned char *b, size_t lb)
{
	size_t m = la < lb ? la : lb;
	int r = memcmp(a, b, m);
	if (r != 0) return r;
	if (la == lb) return 0;
	return la < lb ? -1 : 1;
}

void radix_init(radix_root_t *tree,
				const unsigned char *(*get_key)(const struct radix_leaf *leaf, size_t *len, void *arg),
				void *key_arg,
				void (*leaf_destructor)(struct radix_leaf *leaf, void *arg),
				void *destructor_arg)
{
	tree->root = NULL; // 延迟创建根
	tree->height = 0;
	tree->gfp_mask = 0;
	tree->get_key = get_key;
	tree->key_arg = key_arg;
	tree->leaf_destructor = leaf_destructor;
	tree->destructor_arg = destructor_arg;
}

// Linux内核风格初始化
void radix_tree_init(radix_root_t *root)
{
	root->root = NULL;
	root->height = 0;
	root->gfp_mask = 0;
	root->get_key = NULL;
	root->key_arg = NULL;
	root->leaf_destructor = NULL;
	root->destructor_arg = NULL;
}

static struct radix_node *ensure_root(radix_root_t *tree)
{
	if (!tree->root) {
		tree->root = radix_node_new(NULL, 0);
	}
	return tree->root;
}

int radix_empty(const radix_root_t *tree)
{
	return !tree->root || (!tree->root->leaf && !tree->root->first_child);
}

// 在节点 n 上按 key 继续向下，可能产生分裂；
// 返回：
// - 到达的节点（可能是中间或叶终止点）
// - *key 和 *len 会前移/减少
static struct radix_node *descend_and_split(struct radix_node *n,
											const unsigned char **key, size_t *len)
{
	while (*len > 0) {
		struct radix_node *child = find_child_by_first(n, **key);
		if (!child) return n; // 在此处创建新的分支

		// 与子标签比较
		size_t lcp = common_prefix(child->label, child->label_len, *key, *len);
		if (lcp == child->label_len) {
			// 完全匹配子标签，进入子节点
			*key += lcp; *len -= lcp;
			n = child;
			continue;
		}

		// 需要分裂 child，将其标签分为 [0..lcp) 作为新中间节点 remainder 作为其子
		struct radix_node *mid = radix_node_new(child->label, lcp);
		if (!mid) return NULL;

		// 调整链接：把 child 替换为 mid
		mid->parent = (struct radix_node *)n;
		// 修复父的 child 链接
		if (n->first_child == child) {
			mid->next_sibling = child->next_sibling;
			n->first_child = mid;
		} else {
			struct radix_node *p = n->first_child;
			while (p && p->next_sibling != child) p = p->next_sibling;
			if (p) {
				mid->next_sibling = child->next_sibling;
				p->next_sibling = mid;
			}
		}

		// child 去掉前缀，成为 mid 的孩子
		size_t remain = child->label_len - lcp;
		unsigned char *newlab = (unsigned char *)malloc(remain);
		if (!newlab && remain) { radix_node_free(mid); return NULL; }
		if (remain) memcpy(newlab, child->label + lcp, remain);
		free(child->label);
		child->label = newlab;
		child->label_len = remain;

		// 重新链接 child 到 mid
		child->parent = mid;
		child->next_sibling = NULL;
		mid->first_child = child;

		// mid 不继承 leaf；保持 child->leaf 不变

		// 现在判断与 mid 的匹配（lcp == mid->label_len）
		*key += lcp; *len -= lcp;
		return mid; // 让上层循环从 mid 继续插入新分支
	}
	return n;
}

int radix_insert(radix_root_t *tree, struct radix_leaf *leaf)
{
	if (!tree || !leaf || !tree->get_key) return -2;
	if (leaf->term) return -1; // 已在树中

	size_t len = 0; const unsigned char *key = tree->get_key(leaf, &len, tree->key_arg);
	if (!key) return -2;

	struct radix_node *root = ensure_root(tree);
	if (!root) return -2;

	const unsigned char *k = key; size_t l = len;
	struct radix_node *n = descend_and_split(root, &k, &l);
	if (!n) return -2;

	if (l == 0) {
		// 插入在 n 上
		if (n->leaf) return -1; // 重复
		n->leaf = leaf;
		leaf->term = n;
		return 0;
	}

	// 在 n 下创建新分支链（单边路径 + 终止）
	struct radix_node *child = find_child_by_first(n, *k);
	if (child) {
		// 未完全消费但 find_child_by_first 命中意味着需要再走一次循环；
		// 这里调用 descend_and_split 再次处理
		n = descend_and_split(n, &k, &l);
		if (!n) return -2;
		if (l == 0) {
			if (n->leaf) return -1;
			n->leaf = leaf;
			leaf->term = n;
			return 0;
		}
	}

	struct radix_node *path = radix_node_new(k, l);
	if (!path) return -2;
	add_child(n, path);
	path->leaf = leaf;
	leaf->term = path;
	return 0;
}

// 顺着树匹配 key（不创建/不分裂），返回：到达的节点以及剩余 key
static struct radix_node *walk_match(const struct radix_node *start,
									 const unsigned char **key, size_t *len)
{
	const struct radix_node *n = start;
	while (n && *len > 0) {
		struct radix_node *child = find_child_by_first(n, **key);
		if (!child) break;
		size_t lcp = common_prefix(child->label, child->label_len, *key, *len);
		if (lcp < child->label_len) break; // 不能完全匹配子标签
		*key += lcp; *len -= lcp;
		n = child;
	}
	return (struct radix_node *)n;
}

struct radix_leaf *radix_search_key(const radix_root_t *tree, const unsigned char *key, size_t len)
{
	if (!tree || !tree->root) return NULL;
	const unsigned char *k = key; size_t l = len;
	struct radix_node *n = walk_match(tree->root, &k, &l);
	if (!n) return NULL;
	if (l == 0) return n->leaf; // 精确落在节点
	return NULL; // 还有剩余字节，说明不存在
}

struct radix_leaf *radix_search_by_leaf(const radix_root_t *tree, const struct radix_leaf *probe)
{
	if (!tree || !probe || !tree->get_key) return NULL;
	size_t len = 0; const unsigned char *key = tree->get_key(probe, &len, tree->key_arg);
	return radix_search_key(tree, key, len);
}

// 若节点无 leaf 且无子，则可删除该节点并向上合并
static void try_prune_up(radix_root_t *tree, struct radix_node *n)
{
	while (n && n->parent) {
		if (n->leaf || n->first_child) break;
		struct radix_node *parent = n->parent;
		// 从父的子链移除 n
		if (parent->first_child == n) parent->first_child = n->next_sibling;
		else {
			struct radix_node *p = parent->first_child;
			while (p && p->next_sibling != n) p = p->next_sibling;
			if (p) p->next_sibling = n->next_sibling;
		}
		radix_node_free(n);
		n = parent;

		// 合并：若 parent 只有一个子且自身不是终止，则把子上提（路径压缩）
		if (n && !n->leaf && n->first_child && !n->first_child->next_sibling) {
			struct radix_node *ch = n->first_child;
			// 连接标签 n.label + ch.label 到 ch 上，然后用 ch 替代 n
			size_t newlen = n->label_len + ch->label_len;
			unsigned char *newlab = NULL;
			if (newlen) {
				newlab = (unsigned char *)malloc(newlen);
				if (!newlab) break; // 放弃合并
				if (n->label_len) memcpy(newlab, n->label, n->label_len);
				if (ch->label_len) memcpy(newlab + n->label_len, ch->label, ch->label_len);
			}
			// n 替换为 ch：把 ch 提到 n 位置
			struct radix_node *parent2 = n->parent;
			if (parent2) {
				if (parent2->first_child == n) parent2->first_child = ch;
				else {
					struct radix_node *p = parent2->first_child;
					while (p && p->next_sibling != n) p = p->next_sibling;
					if (p) p->next_sibling = ch;
				}
			} else {
				tree->root = ch;
			}
			ch->parent = parent2;
			ch->next_sibling = n->next_sibling;

			// 更新 ch 的标签为合并后的
			free(ch->label);
			ch->label = newlab;
			ch->label_len = newlen;

			// 释放 n
			if (n->label) free(n->label);
			free(n);

			// 继续向上
			n = ch->parent;
		}
	}
}

int radix_erase(radix_root_t *tree, struct radix_leaf *leaf)
{
	if (!tree || !leaf || !leaf->term) return -1;
	struct radix_node *n = leaf->term;
	if (n->leaf != leaf) return -1;
	n->leaf = NULL;
	leaf->term = NULL;

	// 修剪空路径与可能的合并
	try_prune_up(tree, n);

	// 如果根已空，释放根
	if (tree->root && !tree->root->leaf && !tree->root->first_child) {
		radix_node_free(tree->root);
		tree->root = NULL;
	}
	return 0;
}

/* removed radix_clear: prefer radix_destroy to avoid leak-prone semantics */

static void destroy_recursive(radix_root_t *tree, struct radix_node *n)
{
	if (!n) return;
	for (struct radix_node *ch = n->first_child; ch; ) {
		struct radix_node *next = ch->next_sibling;
		destroy_recursive(tree, ch);
		ch = next;
	}
	if (n->leaf) {
		// 由库负责断开关联，用户析构仅关注资源释放
		struct radix_leaf *lf = n->leaf;
		lf->term = NULL;
		if (tree->leaf_destructor) {
			tree->leaf_destructor(lf, tree->destructor_arg);
		}
		n->leaf = NULL;
	}
	radix_node_free(n);
}

void radix_destroy(radix_root_t *tree)
{
	if (!tree || !tree->root) return;
	destroy_recursive(tree, tree->root);
	tree->root = NULL;
}

// 找到以 n 为根的最小叶子（字典序）
static struct radix_node *subtree_min_node(struct radix_node *n)
{
    while (n) {
        // 如果当前节点有叶子，它就是最小的（前缀优先）
        if (n->leaf) return n;
        
        // 否则找到标签字典序最小的子节点
        if (!n->first_child) return NULL;
        
        struct radix_node *best = NULL;
        for (struct radix_node *ch = n->first_child; ch; ch = ch->next_sibling) {
            if (!best) best = ch;
            else if (cmp_label(ch->label, ch->label_len, best->label, best->label_len) < 0)
                best = ch;
        }
        n = best;
    }
    return NULL;
}

// 找到以 n 为根的最大叶子
static struct radix_node *subtree_max_node(struct radix_node *n)
{
    while (n) {
        // 选择标签字典序最大的子节点继续向下
        struct radix_node *best = NULL;
        for (struct radix_node *ch = n->first_child; ch; ch = ch->next_sibling) {
            if (!best) best = ch;
            else if (cmp_label(ch->label, ch->label_len, best->label, best->label_len) > 0)
                best = ch;
        }
        
        if (!best) {
            // 没有子节点了，返回当前节点的叶子（如果有）
            return n->leaf ? n : NULL;
        }
        
        // 继续向最大的子节点
        n = best;
    }
    return NULL;
}

static struct radix_node *parent_next_branch(struct radix_node *n)
{
	// 返回向上回溯后，在父的子集中严格大于当前标签的最小分支
	while (n && n->parent) {
		struct radix_node *p = n->parent;
		struct radix_node *next = NULL;
		for (struct radix_node *s = p->first_child; s; s = s->next_sibling) {
			if (s == n) continue;
			int cr = cmp_label(s->label, s->label_len, n->label, n->label_len);
			if (cr > 0 && (!next || cmp_label(s->label, s->label_len, next->label, next->label_len) < 0))
				next = s;
		}
		if (next) return next;
		n = p; // 向上继续
	}
	return NULL;
}

static struct radix_node *parent_prev_branch(struct radix_node *n)
{
	while (n && n->parent) {
		struct radix_node *p = n->parent;
		struct radix_node *prev = NULL;
		for (struct radix_node *s = p->first_child; s; s = s->next_sibling) {
			if (s == n) continue;
			int cr = cmp_label(s->label, s->label_len, n->label, n->label_len);
			if (cr < 0 && (!prev || cmp_label(s->label, s->label_len, prev->label, prev->label_len) > 0))
				prev = s;
		}
		if (prev) return prev;
		n = p;
	}
	return NULL;
}

struct radix_leaf *radix_first(const radix_root_t *tree)
{
    if (!tree || !tree->root) return NULL;
    struct radix_node *n = subtree_min_node(tree->root);
    return n ? n->leaf : NULL;
}

struct radix_leaf *radix_last(const radix_root_t *tree)
{
    if (!tree || !tree->root) return NULL;
    struct radix_node *n = subtree_max_node(tree->root);
    return n ? n->leaf : NULL;
}

struct radix_leaf *radix_next(const struct radix_leaf *leaf)
{
    if (!leaf || !leaf->term) return NULL;
    struct radix_node *n = leaf->term;
    
    // 1. 如果当前节点有子节点，下一个就是子树中的最小节点
    if (n->first_child) {
        // 不能直接用 first_child，需要找字典序最小的子节点
        struct radix_node *min_child = NULL;
        for (struct radix_node *ch = n->first_child; ch; ch = ch->next_sibling) {
            if (!min_child || cmp_label(ch->label, ch->label_len, min_child->label, min_child->label_len) < 0) {
                min_child = ch;
            }
        }
        if (min_child) {
            struct radix_node *m = subtree_min_node(min_child);
            return m ? m->leaf : NULL;
        }
    }
    
    // 2. 向上寻找下一个分支
    struct radix_node *cur = n;
    while (cur && cur->parent) {
        // 在当前节点的兄弟中寻找下一个
        struct radix_node *next_sibling = parent_next_branch(cur);
        if (next_sibling) {
            struct radix_node *m = subtree_min_node(next_sibling);
            return m ? m->leaf : NULL;
        }
        cur = cur->parent;
    }
    
    return NULL;
}

struct radix_leaf *radix_prev(const struct radix_leaf *leaf)
{
    if (!leaf || !leaf->term) return NULL;
    struct radix_node *n = leaf->term;
    
    // 向上寻找前驱
    struct radix_node *cur = n;
    while (cur && cur->parent) {
        // 1. 寻找前驱兄弟分支，取其最大叶子
        struct radix_node *prev_sibling = parent_prev_branch(cur);
        if (prev_sibling) {
            struct radix_node *m = subtree_max_node(prev_sibling);
            return m ? m->leaf : NULL;
        }
        
        // 2. 如果没有前驱兄弟，检查父节点是否有叶子
        if (cur->parent && cur->parent->leaf) {
            return cur->parent->leaf;
        }
        
        cur = cur->parent;
    }
    
    return NULL;
}

// 调试函数：打印树结构
static void debug_tree_recursive(struct radix_node *n, int depth) {
    if (!n) return;
    
    for (int i = 0; i < depth; i++) printf("  ");
    printf("node label=\"");
    for (size_t i = 0; i < n->label_len; i++) {
        if (n->label[i] >= 32 && n->label[i] < 127) {
            printf("%c", n->label[i]);
        } else {
            printf("\\x%02x", n->label[i]);
        }
    }
    printf("\" leaf=%s\n", n->leaf ? "YES" : "NO");
    
    for (struct radix_node *ch = n->first_child; ch; ch = ch->next_sibling) {
        debug_tree_recursive(ch, depth + 1);
    }
}

void radix_debug_tree(const radix_root_t *tree) {
    printf("=== Radix Tree Structure ===\n");
    if (!tree || !tree->root) {
        printf("Empty tree\n");
        return;
    }
    debug_tree_recursive(tree->root, 0);
    printf("========================\n");
}

// ============================================================================
// Linux内核风格API实现
// ============================================================================

// 扩展树的高度以适应索引
static int radix_tree_extend(radix_root_t *root, unsigned long index)
{
	struct radix_node *node;
	unsigned int maxshift;
	
	/* 计算需要的高度 */
	maxshift = 0;
	if (index > 0) {
		for (unsigned long tmp = index; tmp; tmp >>= RADIX_TREE_MAP_SHIFT)
			maxshift += RADIX_TREE_MAP_SHIFT;
	}
	
	/* 扩展到所需高度 */
	while (root->height * RADIX_TREE_MAP_SHIFT < maxshift) {
		node = radix_node_new_fixed(root->height * RADIX_TREE_MAP_SHIFT);
		if (!node)
			return -1;
			
		/* 增加高度 */
		if (root->root) {
			node->slots[0] = node_to_entry(root->root);
			node->count = 1;
		}
		
		root->root = node;
		root->height++;
	}
	
	return 0;
}

// 查找或创建槽位
static void **radix_tree_lookup_slot_internal(radix_root_t *root, unsigned long index, int create)
{
	struct radix_node *node;
	unsigned int shift;
	void **slot;
	
	/* 确保树足够高 */
	if (create && radix_tree_extend(root, index) < 0)
		return NULL;
		
	/* 空树 */
	if (!root->root) {
		if (!create)
			return NULL;
		/* 创建根节点 */
		root->root = radix_node_new_fixed(0);
		if (!root->root)
			return NULL;
		root->height = 1;
	}
	
	node = root->root;
	shift = (root->height - 1) * RADIX_TREE_MAP_SHIFT;
	
	/* 向下遍历到叶子层 */
	while (shift > 0) {
		if (!node->is_fixed_mode) {
			return NULL; /* 不支持混合模式 */
		}
		
		slot = &node->slots[radix_tree_index(index, shift)];
		shift -= RADIX_TREE_MAP_SHIFT;
		
		if (*slot) {
			if (radix_tree_is_internal_node(*slot)) {
				node = entry_to_node(*slot);
			} else {
				/* 存在数据，但需要更深层级 */
				if (create) {
					return NULL; /* 暂不处理数据冲突 */
				}
				return NULL;
			}
		} else if (create) {
			/* 创建新的内部节点 */
			struct radix_node *new_node = radix_node_new_fixed(shift);
			if (!new_node)
				return NULL;
			*slot = node_to_entry(new_node);
			node->count++;
			node = new_node;
		} else {
			return NULL;
		}
	}
	
	/* 现在在叶子层 */
	return &node->slots[radix_tree_index(index, 0)];
}

int radix_tree_insert(radix_root_t *root, unsigned long index, void *item)
{
	void **slot;
	
	if (!item)
		return -1;
		
	slot = radix_tree_lookup_slot_internal(root, index, 1);
	if (!slot)
		return -1;
		
	if (*slot)
		return -1;  /* 已存在 */
		
	*slot = item;
	return 0;
}

void *radix_tree_lookup(const radix_root_t *root, unsigned long index)
{
	void **slot = radix_tree_lookup_slot((radix_root_t *)root, index);
	return slot ? *slot : NULL;
}

void **radix_tree_lookup_slot(radix_root_t *root, unsigned long index)
{
	return radix_tree_lookup_slot_internal(root, index, 0);
}

void *radix_tree_delete(radix_root_t *root, unsigned long index)
{
	void **slot;
	void *item;
	
	slot = radix_tree_lookup_slot(root, index);
	if (!slot || !*slot)
		return NULL;
		
	item = *slot;
	*slot = NULL;
	
	/* TODO: 收缩树和清理空节点 */
	
	return item;
}

// 标记操作
void *radix_tree_tag_set(radix_root_t *root, unsigned long index, unsigned int tag)
{
	struct radix_node *node;
	unsigned int shift;
	void **slot;
	int offset;
	
	if (tag >= RADIX_TREE_MAX_TAGS)
		return NULL;
		
	slot = radix_tree_lookup_slot(root, index);
	if (!slot || !*slot)
		return NULL;
		
	/* 设置从叶子到根的路径上的标记 */
	shift = root->height * RADIX_TREE_MAP_SHIFT;
	node = root->root;
	
	while (shift > 0) {
		shift -= RADIX_TREE_MAP_SHIFT;
		offset = radix_tree_index(index, shift);
		tag_set(node, tag, offset);
		
		if (shift > 0) {
			void *entry = node->slots[offset];
			if (radix_tree_is_internal_node(entry))
				node = entry_to_node(entry);
			else
				break;
		}
	}
	
	return *slot;
}

void *radix_tree_tag_clear(radix_root_t *root, unsigned long index, unsigned int tag)
{
	struct radix_node *node;
	unsigned int shift;
	void **slot;
	int offset;
	
	if (tag >= RADIX_TREE_MAX_TAGS)
		return NULL;
		
	slot = radix_tree_lookup_slot(root, index);
	if (!slot || !*slot)
		return NULL;
		
	/* 清除标记，并向上传播 */
	shift = root->height * RADIX_TREE_MAP_SHIFT;
	node = root->root;
	
	while (shift > 0) {
		shift -= RADIX_TREE_MAP_SHIFT;
		offset = radix_tree_index(index, shift);
		tag_clear(node, tag, offset);
		
		/* TODO: 检查是否还有其他子树有该标记，如果没有则向上清除 */
		
		if (shift > 0) {
			void *entry = node->slots[offset];
			if (radix_tree_is_internal_node(entry))
				node = entry_to_node(entry);
			else
				break;
		}
	}
	
	return *slot;
}

int radix_tree_tag_get(const radix_root_t *root, unsigned long index, unsigned int tag)
{
	struct radix_node *node;
	unsigned int shift;
	int offset;
	
	if (tag >= RADIX_TREE_MAX_TAGS)
		return 0;
		
	if (!root->root)
		return 0;
		
	shift = root->height * RADIX_TREE_MAP_SHIFT;
	node = root->root;
	
	while (shift > 0) {
		shift -= RADIX_TREE_MAP_SHIFT;
		offset = radix_tree_index(index, shift);
		
		if (!tag_get(node, tag, offset))
			return 0;
			
		if (shift > 0) {
			void *entry = node->slots[offset];
			if (radix_tree_is_internal_node(entry))
				node = entry_to_node(entry);
			else
				return 0;
		}
	}
	
	return 1;
}

int radix_tree_tagged(const radix_root_t *root, unsigned int tag)
{
	if (tag >= RADIX_TREE_MAX_TAGS || !root->root)
		return 0;
	return any_tag_set(root->root, tag);
}

// Gang lookup
unsigned int radix_tree_gang_lookup(const radix_root_t *root, void **results,
				unsigned long first_index, unsigned int max_items)
{
	unsigned int ret = 0;
	unsigned long index = first_index;
	
	while (ret < max_items) {
		void *item = radix_tree_lookup(root, index);
		if (item) {
			results[ret++] = item;
		}
		if (index == ~0UL)  /* 避免溢出 */
			break;
		index++;
	}
	
	return ret;
}

unsigned int radix_tree_gang_lookup_slot(radix_root_t *root, void ***results,
				unsigned long first_index, unsigned int max_items)
{
	unsigned int ret = 0;
	unsigned long index = first_index;
	
	while (ret < max_items) {
		void **slot = radix_tree_lookup_slot(root, index);
		if (slot && *slot) {
			results[ret++] = slot;
		}
		if (index == ~0UL)  /* 避免溢出 */
			break;
		index++;
	}
	
	return ret;
}

unsigned int radix_tree_gang_lookup_tag(const radix_root_t *root, void **results,
			unsigned long first_index, unsigned int max_items, unsigned int tag)
{
	unsigned int ret = 0;
	unsigned long index = first_index;
	
	while (ret < max_items) {
		if (radix_tree_tag_get(root, index, tag)) {
			void *item = radix_tree_lookup(root, index);
			if (item) {
				results[ret++] = item;
			}
		}
		if (index == ~0UL)  /* 避免溢出 */
			break;
		index++;
	}
	
	return ret;
}

unsigned int radix_tree_gang_lookup_tag_slot(radix_root_t *root, void ***results,
			unsigned long first_index, unsigned int max_items, unsigned int tag)
{
	unsigned int ret = 0;
	unsigned long index = first_index;
	
	while (ret < max_items) {
		if (radix_tree_tag_get(root, index, tag)) {
			void **slot = radix_tree_lookup_slot(root, index);
			if (slot && *slot) {
				results[ret++] = slot;
			}
		}
		if (index == ~0UL)  /* 避免溢出 */
			break;
		index++;
	}
	
	return ret;
}

// 统计函数
unsigned long radix_tree_count(const radix_root_t *root)
{
	/* 遍历计数 */
	unsigned long count = 0;
	unsigned long index = 0;
	
	do {
		if (radix_tree_lookup(root, index))
			count++;
		if (index == ~0UL)
			break;
		index++;
	} while (index != 0);  /* 检测回绕 */
	
	return count;
}

unsigned int radix_tree_height(const radix_root_t *root)
{
	return root ? root->height : 0;
}
