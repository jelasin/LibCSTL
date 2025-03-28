#include "list.h"
#include <stdio.h>

// 不小心访问到已经释放的内存
static void* LIST_POSTION1 = (void*)0xdeadbeef;
static void* LIST_POSTION2 = (void*)0xdeadbeef;

static void __list_add(list_node *new, list_node *prev, list_node *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static void __list_del(list_node *node)
{
    node->next->prev = node->prev;
    node->prev->next = node->next;
}

// 初始化链表头
void init_list_head(list_head *list)
{
    list->next = list->prev = list;
}

// 插入节点到当前节点之前
void list_add_prev(list_node *node, list_node *cur)
{
    __list_add(node, cur->prev, cur);
}

// 插入节点到当前节点之后
void list_add_next(list_node *node, list_node *cur)
{
    __list_add(node, cur, cur->next);
}

// 链表头插入元素
void list_add_head(list_node *node, list_head *head)
{
    __list_add(node, head, head->next);
}

// 链表尾插入元素
void list_add_tail(list_node *node, list_head *head)
{
    __list_add(node, head->prev, head);
}

// 链表删除元素
void list_del(list_node *node)
{
    __list_del(node);
    node->next = LIST_POSTION1;
    node->prev = LIST_POSTION2;
}

// 链表判断是否为空
int list_empty(const list_head *head)
{
    return head->next == head;
}