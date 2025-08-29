#include "hlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 8

struct person {
    hlist_node node;  // 哈希表节点
    int id;
    char name[32];
};

// 简单的哈希函数
static unsigned int hash_func(int key) 
{
    return key % HASH_SIZE;
}

int main() 
{
    // 创建哈希表
    hlist_head hash_table[HASH_SIZE];
    int i;
    
    // 初始化哈希表头
    for (i = 0; i < HASH_SIZE; i++) 
    {
        hlist_init_head(&hash_table[i]);
    }
    
    // 创建一些数据
    struct person p1 = {.id = 1, .name = "Alice"};
    struct person p2 = {.id = 9, .name = "Bob"};    // hash到相同位置(9%8=1)
    struct person p3 = {.id = 5, .name = "Charlie"};
    struct person p4 = {.id = 13, .name = "David"}; // hash到相同位置(13%8=5)
    
    // 插入哈希表
    hlist_add_head(&p1.node, &hash_table[hash_func(p1.id)]);
    hlist_add_head(&p2.node, &hash_table[hash_func(p2.id)]);
    hlist_add_head(&p3.node, &hash_table[hash_func(p3.id)]);
    hlist_add_head(&p4.node, &hash_table[hash_func(p4.id)]);
    
    printf("哈希表内容:\n");
    for (i = 0; i < HASH_SIZE; i++) 
    {
        printf("桶 %d: ", i);
        if (hlist_empty(&hash_table[i])) 
        {
            printf("空\n");
            continue;
        }
        
        struct person *pos;
        hlist_for_each_entry(pos, &hash_table[i], node) 
        {
            printf("(%d, %s) ", pos->id, pos->name);
        }
        printf("\n");
    }
    
    // 查找id为9的人
    int id_to_find = 9;
    unsigned int hash = hash_func(id_to_find);
    struct person *found = NULL;
    struct person *pos;
    
    hlist_for_each_entry(pos, &hash_table[hash], node) 
    {
        if (pos->id == id_to_find) 
        {
            found = pos;
            break;
        }
    }
    
    if (found) 
    {
        printf("\n找到: ID=%d, Name=%s\n", found->id, found->name);
        
        // 删除找到的节点
        printf("删除 ID=%d\n", found->id);
        hlist_del(&found->node);
    } 
    else 
    {
        printf("\nID=%d 未找到\n", id_to_find);
    }
    
    // 再次打印哈希表内容
    printf("\n删除后的哈希表内容:\n");
    for (i = 0; i < HASH_SIZE; i++) 
    {
        printf("桶 %d: ", i);
        if (hlist_empty(&hash_table[i])) 
        {
            printf("空\n");
            continue;
        }
        
        struct person *pos;
        hlist_for_each_entry(pos, &hash_table[i], node) 
        {
            printf("(%d, %s) ", pos->id, pos->name);
        }
        printf("\n");
    }
    
    return 0;
}

// 哈希表内容:
// 桶 0: 空
// 桶 1: (9, Bob) (1, Alice) 
// 桶 2: 空
// 桶 3: 空
// 桶 4: 空
// 桶 5: (13, David) (5, Charlie) 
// 桶 6: 空
// 桶 7: 空

// 找到: ID=9, Name=Bob
// 删除 ID=9

// 删除后的哈希表内容:
// 桶 0: 空
// 桶 1: (1, Alice) 
// 桶 2: 空
// 桶 3: 空
// 桶 4: 空
// 桶 5: (13, David) (5, Charlie) 
// 桶 6: 空
// 桶 7: 空