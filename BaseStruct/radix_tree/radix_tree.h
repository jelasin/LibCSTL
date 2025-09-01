#ifndef __RADIX_TREE_H__
#define __RADIX_TREE_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef offsetof
typedef unsigned long size_t;
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
#  if defined(__GNUC__)
#    define container_of(ptr, type, member) ({                                          \
			const __typeof__(((type *)0)->member) *__mptr =                             \
				(const __typeof__(((type *)0)->member) *)(ptr);                         \
			(type *)((char *)__mptr - offsetof(type, member));                          \
		})
#  else
#    define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))
#  endif
#endif

#ifndef radix_entry
#define radix_entry(ptr, type, member) \
	container_of(ptr, type, member)
#endif

// 基数树配置（参考Linux内核设计）
#define RADIX_TREE_MAP_SHIFT	6
#define RADIX_TREE_MAP_SIZE	(1UL << RADIX_TREE_MAP_SHIFT)
#define RADIX_TREE_MAP_MASK	(RADIX_TREE_MAP_SIZE-1)
#define RADIX_TREE_TAG_LONGS	\
	((RADIX_TREE_MAP_SIZE + BITS_PER_LONG - 1) / BITS_PER_LONG)
#define RADIX_TREE_MAX_TAGS	3

#ifndef BITS_PER_LONG
#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#endif

// 标记枚举（参考内核）
enum {
	RADIX_TREE_TAG_DIRTY = 0,
	RADIX_TREE_TAG_WRITEBACK = 1,
	RADIX_TREE_TAG_TOWRITE = 2
};

// 基数树（Radix Tree，支持整数键索引和字节序列两种模式）
// 设计说明：
// - 支持Linux内核风格的整数键索引（固定64叉树）
// - 保留原有的intrusive字节序列模式用于兼容性
// - 支持标记（tags）功能
// - 支持批量查找（gang lookup）

// 前向声明
struct radix_node;

// 叶子（用户结构内嵌的成员，表示一个键）
struct radix_leaf {
	// 指向其终止的树节点，便于删除与遍历
	struct radix_node *term;   // NULL 表示未插入
};

// 基数树根
typedef struct radix_root {
	struct radix_node *root;   // 根节点（内部节点，label_len==0）
	unsigned int height;       // 树的高度（用于整数键模式）
	unsigned long gfp_mask;    // 内存分配标志（保留）

	// 键提取回调：从叶子获取键的指针与长度（字节序列模式）
	// 返回值：键的起始地址；必须在叶子生命周期内保持有效
	const unsigned char *(*get_key)(const struct radix_leaf *leaf, size_t *len, void *arg);
	void *key_arg;             // 键回调的用户参数

	// 叶子析构回调：用于 destroy 时释放用户节点（如整节点 free）
	void (*leaf_destructor)(struct radix_leaf *leaf, void *arg);
	void *destructor_arg;      // 析构回调的用户参数
} radix_root_t;

// 初始化宏（静态初始化）
#define RADIX_TREE_INIT(mask) { \
	.root = NULL, \
	.height = 0, \
	.gfp_mask = (mask), \
	.get_key = NULL, \
	.key_arg = NULL, \
	.leaf_destructor = NULL, \
	.destructor_arg = NULL \
}

#define RADIX_TREE(name, mask) \
	struct radix_root name = RADIX_TREE_INIT(mask)

// ============================================================================
// 传统字节序列API（保持兼容性）
// ============================================================================

// 初始化
extern void radix_init(radix_root_t *tree,
					   const unsigned char *(*get_key)(const struct radix_leaf *leaf, size_t *len, void *arg),
					   void *key_arg,
					   void (*leaf_destructor)(struct radix_leaf *leaf, void *arg),
					   void *destructor_arg);

// 插入叶子（键由回调从 leaf 提取）
// 返回 0 成功，-1 已存在（键重复），-2 参数错误/内存不足
extern int radix_insert(radix_root_t *tree, struct radix_leaf *leaf);

// 通过叶子（probe）查找同键的已插入叶子
extern struct radix_leaf *radix_search_by_leaf(const radix_root_t *tree, const struct radix_leaf *probe);

// 通过原始键查找
extern struct radix_leaf *radix_search_key(const radix_root_t *tree, const unsigned char *key, size_t len);

// 删除已插入的叶子（使用 leaf->term 直接定位）
// 返回 0 成功，-1 未在树内
extern int radix_erase(radix_root_t *tree, struct radix_leaf *leaf);

// ============================================================================
// Linux内核风格整数键API
// ============================================================================

// 整数键模式初始化
extern void radix_tree_init(radix_root_t *root);

// 插入项目到指定索引
extern int radix_tree_insert(radix_root_t *root, unsigned long index, void *item);

// 查找项目
extern void *radix_tree_lookup(const radix_root_t *root, unsigned long index);

// 查找并返回槽位指针（可用于修改）
extern void **radix_tree_lookup_slot(radix_root_t *root, unsigned long index);

// 删除项目
extern void *radix_tree_delete(radix_root_t *root, unsigned long index);

// 批量查找（gang lookup）
extern unsigned int radix_tree_gang_lookup(const radix_root_t *root,
					void **results, unsigned long first_index,
					unsigned int max_items);

extern unsigned int radix_tree_gang_lookup_slot(radix_root_t *root,
					void ***results, unsigned long first_index,
					unsigned int max_items);

// ============================================================================
// 标记操作（Tags）
// ============================================================================

// 设置标记
extern void *radix_tree_tag_set(radix_root_t *root, unsigned long index, 
				unsigned int tag);

// 清除标记
extern void *radix_tree_tag_clear(radix_root_t *root, unsigned long index,
				unsigned int tag);

// 获取标记状态
extern int radix_tree_tag_get(const radix_root_t *root, unsigned long index,
				unsigned int tag);

// 检查是否有指定标记的项目
extern int radix_tree_tagged(const radix_root_t *root, unsigned int tag);

// 基于标记的批量查找
extern unsigned int radix_tree_gang_lookup_tag(const radix_root_t *root,
				void **results, unsigned long first_index,
				unsigned int max_items, unsigned int tag);

extern unsigned int radix_tree_gang_lookup_tag_slot(radix_root_t *root,
				void ***results, unsigned long first_index,
				unsigned int max_items, unsigned int tag);

// ============================================================================
// 通用操作
// ============================================================================

// 是否为空
extern int radix_empty(const radix_root_t *tree);

// 清空（释放所有内部节点，断开叶子但不调用 leaf_destructor）
extern void radix_clear(radix_root_t *tree);

// 销毁（释放所有内部节点，并对每个叶子调用 leaf_destructor）
extern void radix_destroy(radix_root_t *tree);

// ============================================================================
// 遍历API（字节序列模式）
// ============================================================================

// 遍历（按字节字典序）
extern struct radix_leaf *radix_first(const radix_root_t *tree);
extern struct radix_leaf *radix_last(const radix_root_t *tree);
extern struct radix_leaf *radix_next(const struct radix_leaf *leaf);
extern struct radix_leaf *radix_prev(const struct radix_leaf *leaf);

// 遍历宏（字典序遍历所有叶子）
#define radix_for_each(pos, tree, type, member) \
    for (pos = radix_entry(radix_first(tree), type, member); \
         &pos->member != NULL; \
         pos = radix_entry(radix_next(&pos->member), type, member))

// ============================================================================
// 调试和扩展
// ============================================================================

// 调试函数：打印树结构
extern void radix_debug_tree(const radix_root_t *tree);

// 统计信息
extern unsigned long radix_tree_count(const radix_root_t *root);
extern unsigned int radix_tree_height(const radix_root_t *root);

#endif // __RADIX_TREE_H__