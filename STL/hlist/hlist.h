#ifndef __HLIST_H__
#define __HLIST_H__

#ifndef offsetof
typedef unsigned long size_t;
// 获取结构体成员偏移，常量指针的值为0，可视为结构体首地址为0
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
/* ptr 成员指针
 * type 结构体类型
 * member 成员变量名
 */
#define container_of(ptr, type, member) ({          \
        const typeof( ((type *)0)->member ) *__mptr = (const typeof( ((type *)0)->member ) *)(ptr); \
        (type *)( (char *)__mptr - offsetof(type, member) );})
#endif

// 哈希表节点定义
struct hlist_node {
    struct hlist_node *next, **pprev;
};

// 哈希表头定义
struct hlist_head {
    struct hlist_node *first;
};

typedef struct hlist_node hlist_node;
typedef struct hlist_head hlist_head;

#ifndef hlist_entry
#define hlist_entry(ptr, type, member) \
    container_of(ptr, type, member)
#endif

// 初始化哈希表头
extern void hlist_init_head(hlist_head *head);

// 在头部添加节点
extern void hlist_add_head(hlist_node *node, hlist_head *head);

// 在指定节点后添加节点
extern void hlist_add_after(hlist_node *node, hlist_node *prev);

// 在指定节点前添加节点
extern void hlist_add_before(hlist_node *node, hlist_node *next);

// 从哈希表中删除节点
extern void hlist_del(hlist_node *node);

// 判断哈希表是否为空
extern int hlist_empty(const hlist_head *head);

// 遍历哈希表
#define hlist_for_each(pos, head) \
    for (pos = (head)->first; pos; pos = pos->next)

// 安全遍历哈希表，允许删除
#define hlist_for_each_safe(pos, n, head) \
    for (pos = (head)->first; pos && ({ n = pos->next; 1; }); pos = n)

// 遍历哈希表，获取结构体指针
#define hlist_for_each_entry(pos, head, member) \
    for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member); \
         pos; \
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

// 安全的哈希表遍历，获取结构体指针，允许删除
#define hlist_for_each_entry_safe(pos, n, head, member) \
    for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member); \
         pos && ({ n = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member); 1; }); \
         pos = n)

// 安全获取哈希表节点对应的结构体指针
#define hlist_entry_safe(ptr, type, member) \
    ({ typeof(ptr) ____ptr = (ptr); \
       ____ptr ? hlist_entry(____ptr, type, member) : NULL; \
    })

#endif // __HLIST_H__