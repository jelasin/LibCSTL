# C STL库

打算重写以前学习数据结构时写的[代码](https://jelasin.github.io/2025/01/03/%E6%95%B0%E6%8D%AE%E7%BB%93%E6%9E%84%E5%8F%8A%E7%AE%97%E6%B3%95/)，原版代码的扩展性并不好，实用性也不强。参考`Linux内核`的一些优秀数据结构和算法设计了`C STL`库，里面包含了一些扩展性和移植性较好的数据结构和算法，相较于原版代码，去除了不实用的功能，精简代码，最大程度还原数据结构本质功能, 扩展性强。相较于内核，没有大量使用static inline, 增加了更多的错误处理。

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

### 上层容器实现

#### v1

v1 设计十分简洁.包含一些基础的数据结构, 可以根据自己的需求进行扩展.

- [x] list : 双向链表, 同底层数据结构list.
- [x] hlist : 哈希链表, 同底层数据结构hlist.
- [x] queue : 单调队列, 需要依赖 list.
- [x] deque : 双端队列, 需要依赖 list.
- [x] stack : 单调栈, 需要依赖 list.

#### v2

v2 设计实现更加丰富, 接口更多, 可以进行自定义操作.
支持 malloc_hook 和 free_hook, 可以进行自定义内存管理.

- [x] priority_queue : 优先队列, 基于二叉堆数组实现.
- [x] ring_queue : 环形队列.
- [ ] stack : 单调栈, 需要依赖 list.
- [ ] queue : 单调队列, 需要依赖 list.
- [ ] deque : 双端队列, 需要依赖 list.
- [ ] list : 双向链表, 需要以来list.
- [ ] set : 集合, 需要依赖 rb_tree.
- [ ] map : 映射, 需要依赖 rb_tree.
- [ ] vector : 向量, 需要依赖 list.
- [ ] string : 字符串, 需要依赖 list.

#### v3

v3 设计实现更加复杂一些的数据结构, 包括常用的线程池, 内存池,  进程池, 协程库等设计, 可以进行自定义操作.
这些项目大部分都有独立的仓库, 可以单独使用.

- [x] [LibCoroutine](https://github.com/jelasin/LibCoroutine) : 协程库.
- [ ] LibThreadPool : 线程池.
- [ ] LibMemoryPool : 内存池.
- [ ] LibProcessPool : 进程池.
- [ ] LibSRF : 共享环形队列(POSIX IPC).

### 算法

- [x] sort : 智能排序算法.
- [ ] hash : 哈希算法库.
- [ ] crypto : 加密算法库.
