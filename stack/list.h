#ifndef __LIST_H__
#define __LIST_H__

#ifndef offsetof
// 获取结构体成员偏移，因为常量指针的值为0，即可以看作结构体首地址为0
#define offsetof(TYPE,MEMBER)((unsigned int)&((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
/*ptr 成员指针
* type 结构体 比如struct Stu
* member 成员变量，跟指针对应
* */
// 最后一句的意义就是，取结构体某个成员member的地址，减去这个成员在结构体type中的偏移，运算结果就是结构体type的首地址
#define container_of(ptr, type, member) ({          \
        const typeof( ((type *)0)->member ) *__mptr = (const typeof( ((type *)0)->member ) *)(ptr); \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

#ifndef list_entry
// 获取结构体指针的成员变量地址
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)
#endif

struct list_head {
    struct list_head *next, *prev;
};

typedef struct list_head list_head;
typedef struct list_head list_node;

// 初始化链表头
extern void init_list_head(list_head *list);

// 插入节点到当前节点之前
extern void list_add_prev(list_node *node, list_node *cur);

// 插入节点到当前节点之后
extern void list_add_next(list_node *node, list_node *cur);

// 链表头插入元素
extern void list_add_head(list_node *node, list_head *head);

// 链表尾插入元素
extern void list_add_tail(list_node *node, list_head *head);

// 链表删除元素
extern void list_del(list_node *node);

// 链表判断是否为空
extern int list_empty(const list_head *head);

// 链表遍历
// 只进行遍历,不进行危险操作.
#define list_for_each_entry(pos, head, member)                \
    for (pos = list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head);                              \
         pos = list_entry(pos->member.next, typeof(*pos), member))

// 遍历过程中需要删除节点
#define list_for_each_entry_safe(pos, next, head, member)           \
    for (pos = list_entry((head)->next, typeof(*pos), member),   \
         next = list_entry(pos->member.next, typeof(*pos), member);  \
         &pos->member != (head);                                 \
         pos = next, next = list_entry(next->member.next, typeof(*next), member))

#endif // __LIST_H__