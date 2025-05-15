#include "hashmap.h"
#include <stdio.h>
#include <string.h>

// 自定义结构体
typedef struct {
    int id;
    char name[32];
    double score;
} Student;

// 整数哈希函数
unsigned int int_hash(const void *key, size_t key_len, void *userdata) {
    // 简单的整数哈希函数
    int k = *(int*)key;
    return (unsigned int)(k * 2654435761u); // Knuth乘法散列
}

// 整数比较函数
int int_compare(const void *key1, size_t key_len1, const void *key2, size_t key_len2, void *userdata) {
    int k1 = *(int*)key1;
    int k2 = *(int*)key2;
    return k1 - k2;
}

// 整数复制函数
void* int_dup(const void *key, size_t key_len, void *userdata) {
    int *new_key = malloc(sizeof(int));
    if (new_key)
        *new_key = *(int*)key;
    return new_key;
}

// 结构体打印回调
bool print_student(const void *key, size_t key_len, void *value, size_t value_len, void *userdata) {
    int id = *(int*)key;
    Student *student = (Student*)value;
    printf("学号: %d, 姓名: %s, 分数: %.1f\n", id, student->name, student->score);
    return true;
}

int main() {
    // 创建选项
    hashmap_options_t options = {
        .initial_size = 32,
        .load_factor = 0.7f,
        .hash_fn = int_hash,
        .key_compare = int_compare,
        .key_dup = int_dup,
        // 其他的使用默认的值
    };
    
    // 创建哈希表
    hashmap_t *student_map = hashmap_create(&options);
    if (!student_map) {
        printf("创建哈希表失败\n");
        return 1;
    }
    
    // 添加学生记录
    Student students[] = {
        {10001, "张三", 89.5},
        {10002, "李四", 92.0},
        {10003, "王五", 78.5},
        {10004, "赵六", 85.0},
        {10005, "钱七", 95.5}
    };
    
    for (int i = 0; i < 5; i++) {
        int id = students[i].id;
        hashmap_put(student_map, &id, sizeof(id), &students[i], sizeof(Student));
    }
    
    // 查询学生
    int search_id = 10003;
    Student result;
    
    if (hashmap_get(student_map, &search_id, sizeof(search_id), &result, sizeof(result), NULL) == 0) {
        printf("查找结果: 学号 %d, 姓名: %s, 分数: %.1f\n\n", result.id, result.name, result.score);
    } else {
        printf("未找到学号为 %d 的学生\n\n", search_id);
    }
    
    // 遍历所有学生
    printf("所有学生信息:\n");
    hashmap_foreach(student_map, print_student, NULL);
    
    // 修改学生分数
    search_id = 10002;
    if (hashmap_get(student_map, &search_id, sizeof(search_id), &result, sizeof(result), NULL) == 0) {
        result.score = 98.5;
        hashmap_put(student_map, &search_id, sizeof(search_id), &result, sizeof(result));
    }
    
    // 再次查询
    printf("\n修改后的学生信息:\n");
    if (hashmap_get(student_map, &search_id, sizeof(search_id), &result, sizeof(result), NULL) == 0) {
        printf("查找结果: 学号 %d, 姓名: %s, 分数: %.1f\n", result.id, result.name, result.score);
    }
    
    // 销毁哈希表
    hashmap_destroy(student_map);
    
    return 0;
}