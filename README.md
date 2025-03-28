# C STL库

打算重写以前学习数据结构时写的[代码](https://jelasin.github.io/2025/01/03/%E6%95%B0%E6%8D%AE%E7%BB%93%E6%9E%84%E5%8F%8A%E7%AE%97%E6%B3%95/)，原版代码的扩展性并不好，实用性也不强。参考`Linux内核`的一些数据结构设计了`C STL`库，里面包含了一些扩展性和移植性较好的数据结构和算法，相较于原版代码，去除了不实用的功能，精简代码，最小程度还原数据结构本质功能。相较于内核，去除了static inline等这种比较消耗内存的设计，增加了更多的错误处理。

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

- [x] list : 双向链表
- [x] queue : 队列 ==> depand on list
- [x] stack : 栈
- [ ] priority_queue : 优先队列
- [ ] set : 集合
- [ ] map : 映射
  