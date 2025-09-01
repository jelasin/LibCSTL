# C STL库

打算重写以前学习数据结构时写的代码，原版代码的扩展性并不好，实用性也不强。参考`Linux内核`的一些优秀数据结构和算法设计了`C STL`库，里面包含了一些扩展性和移植性较好的数据结构和算法，相较于原版代码，去除了不实用的功能，精简代码，最大程度还原数据结构本质功能, 扩展性强。相较于内核，没有大量使用static inline, 增加了更多的错误处理。

## 目录结构

```sh
example.c
xxx.h
xxx.c
```

- `example.c`：示例代码
- `xxx.h`：头文件
- `xxx.c`：实现文件

## development

### 底层数据结构

- [x] list : 双向链表.
- [x] hlist : 哈希链表.
- [x] lru_list : lru链表, 依赖于hlist和list.
- [x] splay_tree : 伸展树.
- [x] avl_tree : 平衡二叉树.
- [x] rb_tree : 红黑树.
- [x] b_tree : B树.
- [x] radix_tree : 基数树.

### 上层数据结构实现

- [x] list : 双向链表, 同底层数据结构 list.
- [x] hlist : 哈希链表, 同底层数据结构 hlist.
- [x] queue : 单调队列, 需要依赖 list.
- [x] deque : 双端队列, 需要依赖 list.
- [x] stack : 单调栈, 需要依赖 list.
- [x] priority_queue : 优先队列, 基于二叉堆数组实现.
- [x] ring_queue : 环形队列.
- [x] hashmap : 哈希表, 需要依赖 rb_tree.

### 算法

- [x] sort : 智能排序算法.
- - [x] SORT_BUBBLE
- - [x] SORT_SELECTION
- - [x] SORT_INSERTION
- - [x] SORT_QUICK
- - [x] SORT_MERGE
- - [x] SORT_HEAP
- [x] hash : 哈希算法库.
- - [x] APHash
- - [x] BKDRHash
- - [x] DJB2Hash
- - [x] ELFHash
- - [x] JSHash
- - [x] MD5
- - [x] PJWHash
- - [x] RSHash
- - [x] SDBMHash
- - [x] SimpleHash
- [ ] crypto : 加密算法库.
- - [ ] AES
- - [ ] DES
- - [ ] DSA
- - [ ] ECC
- - [ ] RSA
- - [ ] RC4
- - [ ] chacha20
- - [ ] tee
