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

// 哈希链表节点
typedef struct hlist_node {
    struct hlist_node *next;
    struct hlist_node **pprev;
} hlist_node;

// 哈希表头
typedef struct hlist_head {
    struct hlist_node *first;
} hlist_head;

// 初始化哈希表头
void hlist_init_head(hlist_head *head);

// 在头部添加节点
void hlist_add_head(hlist_node *node, hlist_head *head);

// 在指定节点后添加节点
void hlist_add_after(hlist_node *node, hlist_node *prev);

// 在指定节点前添加节点
void hlist_add_before(hlist_node *node, hlist_node *next);

// 从哈希表中删除节点
void hlist_del(hlist_node *node);

// 判断哈希表是否为空
int hlist_empty(const hlist_head *head);

// 获取包含某个字段的结构体实例
#define hlist_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

// 遍历哈希链表
#define hlist_for_each(pos, head) \
    for (pos = (head)->first; pos; pos = pos->next)

// 安全遍历哈希链表（允许删除）
#define hlist_for_each_safe(pos, n, head) \
    for (pos = (head)->first, n = pos ? pos->next : NULL; pos; \
         pos = n, n = pos ? pos->next : NULL)

// 遍历哈希链表中的结构体
#define hlist_for_each_entry(entry, head, member) \
    for (entry = (head)->first ? hlist_entry((head)->first, typeof(*(entry)), member) : NULL; \
         entry; \
         entry = entry->member.next ? hlist_entry(entry->member.next, typeof(*(entry)), member) : NULL)

// 安全遍历哈希链表中的结构体（允许删除）
#define hlist_for_each_entry_safe(entry, n, head, member) \
    for (entry = (head)->first ? hlist_entry((head)->first, typeof(*(entry)), member) : NULL, \
         n = entry ? (entry->member.next ? hlist_entry(entry->member.next, typeof(*(entry)), member) : NULL) : NULL; \
         entry; \
         entry = n, n = entry ? (entry->member.next ? hlist_entry(entry->member.next, typeof(*(entry)), member) : NULL) : NULL)

#endif // __HLIST_H__