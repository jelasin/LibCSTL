#include "list.h"
#include <stdio.h>

// 不小心访问到已经释放的内存
static void* LIST_POSTION1 = (void*)0xdeadbeef;
static void* LIST_POSTION2 = (void*)0xdeadbeef;

static void __list_add(struct list_head *new, struct list_head *prev, struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static void __list_del(struct list_head *entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}

// 初始化链表头
void init_list_head(struct list_head *list)
{
    list->next = list->prev = list;
}

// 链表头插入元素
void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

// 链表尾插入元素
void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}

// 链表删除元素
void list_del(struct list_head *entry)
{
    __list_del(entry);
    entry->next = LIST_POSTION1;
    entry->prev = LIST_POSTION2;
}

// 链表判断是否为空
int list_empty(const struct list_head *head)
{
    return head->next == head;
}