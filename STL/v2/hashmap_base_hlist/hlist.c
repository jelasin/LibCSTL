#include "hlist.h"

// 不小心访问到已经释放的内存
static void* HLIST_POISONED_POINTER = (void*)0xdeadbeef;

// 初始化哈希表头
void hlist_init_head(hlist_head *head)
{
    head->first = (void*)0;
}

// 在头部添加节点
void hlist_add_head(hlist_node *node, hlist_head *head)
{
    struct hlist_node *first = head->first;
    node->next = first;
    
    if (first)
    {
        first->pprev = &node->next;
    }
        
    head->first = node;
    node->pprev = &head->first;
}

// 在指定节点后添加节点
void hlist_add_after(hlist_node *node, hlist_node *prev)
{
    node->next = prev->next;
    node->pprev = &prev->next;
    
    if (prev->next)
    {
        prev->next->pprev = &node->next;
    }
        
    prev->next = node;
}

// 在指定节点前添加节点
void hlist_add_before(hlist_node *node, hlist_node *next)
{
    node->pprev = next->pprev;
    node->next = next;
    *node->pprev = node;
    next->pprev = &node->next;
}

// 从哈希表中删除节点
void hlist_del(hlist_node *node)
{
    struct hlist_node *next = node->next;
    struct hlist_node **pprev = node->pprev;
    
    // 更新前一个节点的next指针
    *pprev = next;
    
    // 如果存在后继节点，更新其pprev指针
    if (next)
    {
        next->pprev = pprev;
    }
    
    // 设置被删除节点的指针为危险值
    node->next = HLIST_POISONED_POINTER;
    node->pprev = HLIST_POISONED_POINTER;
}

// 判断哈希表是否为空
int hlist_empty(const hlist_head *head)
{
    return head->first == (void*)0;
}