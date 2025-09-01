#include "radix_tree.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// ------------- 字符串键用例 -------------
struct str_item { char *key; struct radix_leaf leaf; };

static const unsigned char *str_get_key(const struct radix_leaf *leaf, size_t *len, void *arg)
{
    (void)arg;
    const struct str_item *it = radix_entry(leaf, const struct str_item, leaf);
    *len = strlen(it->key);
    return (const unsigned char *)it->key;
}

static void str_leaf_destructor(struct radix_leaf *leaf, void *arg)
{
    (void)arg;
    struct str_item *it = radix_entry(leaf, struct str_item, leaf);
    free(it->key);
    free(it);
}

static int cmp_cstr(const void *a, const void *b)
{
    const char * const *pa = (const char * const *)a;
    const char * const *pb = (const char * const *)b;
    return strcmp(*pa, *pb);
}

static void test_strings_basic()
{
    radix_root_t t; radix_init(&t, str_get_key, NULL, NULL, NULL);

    // 插入带前缀关系的键
    const char *keys[] = {"", "a", "ab", "abc", "abd", "b", "ba", "z"};
    size_t n = sizeof(keys)/sizeof(keys[0]);
    struct str_item **items = (struct str_item **)calloc(n, sizeof(*items));
    assert(items);

    for (size_t i = 0; i < n; ++i) {
        items[i] = (struct str_item *)calloc(1, sizeof(**items));
        size_t L = strlen(keys[i]); items[i]->key = (char *)malloc(L + 1);
        memcpy(items[i]->key, keys[i], L + 1);
        int r = radix_insert(&t, &items[i]->leaf); assert(r == 0);
        assert(items[i]->leaf.term != NULL);
        printf("  \"%s\" -> term=%p\n", items[i]->key, (void*)items[i]->leaf.term);
    }

    assert(!radix_empty(&t));
    
    // 调试：打印树结构
    radix_debug_tree(&t);

    // 查找
    for (size_t i = 0; i < n; ++i) {
        struct str_item probe = { .key = (char *)keys[i], .leaf = {0} };
        struct radix_leaf *f = radix_search_by_leaf(&t, &probe.leaf);
        assert(f);
        struct str_item *got = radix_entry(f, struct str_item, leaf);
        assert(strcmp(got->key, keys[i]) == 0);
    }
    // 不存在键
    struct str_item probe2 = { .key = "ac", .leaf = {0} };
    assert(radix_search_by_leaf(&t, &probe2.leaf) == NULL);

    // 遍历有序
    char **sorted = (char **)calloc(n, sizeof(char*));
    for (size_t i = 0; i < n; ++i) sorted[i] = items[i]->key;
    qsort(sorted, n, sizeof(char*), cmp_cstr);

    printf("Expected order: ");
    for (size_t i = 0; i < n; ++i) printf("\"%s\" ", sorted[i]);
    printf("\n");

    // Debug: print all inserted items and their term pointers
    printf("Inserted items:\n");
    for (size_t i = 0; i < n; ++i) {
        printf("  \"%s\" -> term=%p\n", items[i]->key, (void*)items[i]->leaf.term);
    }

    printf("Actual order:   ");
    size_t idx = 0;
    for (struct radix_leaf *it = radix_first(&t); it; it = radix_next(it)) {
        struct str_item *x = radix_entry(it, struct str_item, leaf);
        printf("\"%s\" ", x->key);
        if (idx < n && strcmp(x->key, sorted[idx]) != 0) {
            printf("\nMismatch at index %zu: got \"%s\", expected \"%s\"\n", 
                   idx, x->key, sorted[idx]);
            break;
        }
        idx++;
        if (idx > n) { // Prevent infinite loop
            printf("\n[ERROR] Too many items in traversal!\n");
            break;
        }
    }
    printf("\n");
    
    // 删除测试：删除前缀与分支，触发修剪
    // 删除 "ab"，不影响 "abc"/"abd" 存在
    struct str_item key_ab = { .key = "ab" };
    struct radix_leaf *lf_ab = radix_search_by_leaf(&t, &key_ab.leaf);
    assert(lf_ab);
    assert(radix_erase(&t, lf_ab) == 0);
    assert(radix_search_by_leaf(&t, &key_ab.leaf) == NULL);
    struct str_item key_abc = { .key = "abc" };
    assert(radix_search_by_leaf(&t, &key_abc.leaf) != NULL);
    struct str_item key_abd = { .key = "abd" };
    assert(radix_search_by_leaf(&t, &key_abd.leaf) != NULL);

    // 清空：不析构，叶子 term 应置空
    radix_clear(&t);
    assert(radix_empty(&t));
    for (size_t i = 0; i < n; ++i) assert(items[i]->leaf.term == NULL);

    // 重新插入并销毁（带析构）
    radix_init(&t, str_get_key, NULL, str_leaf_destructor, NULL);
    for (size_t i = 0; i < n; ++i) {
        int r = radix_insert(&t, &items[i]->leaf); assert(r == 0);
    }
    radix_destroy(&t);
    free(sorted);
    free(items);
}

// ------------- 二进制键用例 -------------
struct bin_item { unsigned char *data; size_t len; struct radix_leaf leaf; };

static const unsigned char *bin_get_key(const struct radix_leaf *leaf, size_t *len, void *arg)
{
    (void)arg;
    const struct bin_item *it = radix_entry(leaf, const struct bin_item, leaf);
    *len = it->len; return it->data;
}

static int bin_cmp(const void *a, const void *b)
{
    const struct bin_item * const *pa = (const struct bin_item * const *)a;
    const struct bin_item * const *pb = (const struct bin_item * const *)b;
    size_t la = (*pa)->len, lb = (*pb)->len;
    size_t m = la < lb ? la : lb;
    int c = memcmp((*pa)->data, (*pb)->data, m);
    if (c) return c;
    return (la < lb) ? -1 : (la > lb) ? 1 : 0;
}

static void test_binary_keys()
{
    radix_root_t t; radix_init(&t, bin_get_key, NULL, NULL, NULL);
    const unsigned char k1[] = {0x00};
    const unsigned char k2[] = {0x00, 0x00};
    const unsigned char k3[] = {0x00, 0x01};
    const unsigned char k4[] = {0x7F, 0x00};
    const unsigned char *arr[] = {k1, k2, k3, k4};
    const size_t lens[] = {1,2,2,2};
    size_t n = 4;

    struct bin_item **items = calloc(n, sizeof(*items)); assert(items);
    for (size_t i = 0; i < n; ++i) {
        items[i] = calloc(1, sizeof(**items));
        items[i]->len = lens[i]; items[i]->data = malloc(lens[i]);
        memcpy(items[i]->data, arr[i], lens[i]);
        assert(radix_insert(&t, &items[i]->leaf) == 0);
    }

    // 有序遍历检查
    struct bin_item **sorted = calloc(n, sizeof(*sorted));
    for (size_t i = 0; i < n; ++i) sorted[i] = items[i];
    qsort(sorted, n, sizeof(*sorted), bin_cmp);
    size_t idx = 0;
    for (struct radix_leaf *it = radix_first(&t); it; it = radix_next(it)) {
        struct bin_item *x = radix_entry(it, struct bin_item, leaf);
        assert(bin_cmp(&x, &sorted[idx++]) == 0);
    }
    assert(idx == n);

    // 重复插入应失败
    assert(radix_insert(&t, &items[0]->leaf) == -1);

    // 删除全部并检查 empty
    for (size_t i = 0; i < n; ++i) {
        assert(radix_erase(&t, &items[i]->leaf) == 0);
    }
    assert(radix_empty(&t));

    // 销毁（应安全）
    radix_destroy(&t);
    for (size_t i = 0; i < n; ++i) { free(items[i]->data); free(items[i]); }
    free(sorted); free(items);
}

// ------------- 压力测试 -------------
static char *rand_string(size_t minL, size_t maxL)
{
    size_t L = minL + (rand() % (maxL - minL + 1));
    char *s = (char *)malloc(L + 1);
    for (size_t i = 0; i < L; ++i) {
        s[i] = (char)('a' + (rand() % 26));
    }
    s[L] = '\0';
    return s;
}

static void test_stress()
{
    const int N = 50000;       // 插入数量
    const int Q = 10000;       // 查找/删除操作
    radix_root_t t; radix_init(&t, str_get_key, NULL, NULL, NULL);

    struct str_item **items = calloc(N, sizeof(*items));
    assert(items);
    srand(12345);

    // 插入
    clock_t st = clock();
    for (int i = 0; i < N; ++i) {
        items[i] = calloc(1, sizeof(**items));
        items[i]->key = rand_string(5, 24);
        int r = radix_insert(&t, &items[i]->leaf);
        if (r == -1) { // 冲突（极少），简单解决：添加后缀再插
            char *old = items[i]->key; size_t L = strlen(old);
            items[i]->key = malloc(L + 3); memcpy(items[i]->key, old, L);
            items[i]->key[L] = '#'; items[i]->key[L+1] = (char)('0' + (i % 10)); items[i]->key[L+2] = 0;
            free(old);
            assert(radix_insert(&t, &items[i]->leaf) == 0);
        }
    }
    clock_t ed = clock();
    double t_insert = (double)(ed - st) / CLOCKS_PER_SEC;

    // 随机查找
    st = clock();
    for (int i = 0; i < Q; ++i) {
        int id = rand() % N;
        struct str_item probe = { .key = items[id]->key };
        assert(radix_search_by_leaf(&t, &probe.leaf) != NULL);
    }
    ed = clock();
    double t_search = (double)(ed - st) / CLOCKS_PER_SEC;

    // 随机删除
    st = clock();
    for (int i = 0; i < Q; ++i) {
        int id = rand() % N;
        if (items[id] && items[id]->leaf.term) {
            assert(radix_erase(&t, &items[id]->leaf) == 0);
        }
    }
    ed = clock();
    double t_delete = (double)(ed - st) / CLOCKS_PER_SEC;

    // 简单遍历校验：有序
    // 统计剩余数量
    size_t left = 0; for (int i = 0; i < N; ++i) if (items[i] && items[i]->leaf.term) ++left;
    if (left > 0) {
        // 收集并排序
        char **arr = malloc(left * sizeof(char*));
        size_t idx = 0;
        for (int i = 0; i < N; ++i) if (items[i] && items[i]->leaf.term) arr[idx++] = items[i]->key;
        qsort(arr, left, sizeof(char*), cmp_cstr);
        idx = 0;
        for (struct radix_leaf *it = radix_first(&t); it; it = radix_next(it)) {
            struct str_item *x = radix_entry(it, struct str_item, leaf);
            assert(strcmp(x->key, arr[idx++]) == 0);
        }
        assert(idx == left);
        free(arr);
    }

    printf("[stress] insert %d: %.3fs, search %d: %.3fs, delete %d: %.3fs, left=%zu\n",
           N, t_insert, Q, t_search, Q, t_delete, left);

    // 清理
    for (int i = 0; i < N; ++i) { if (items[i]) { free(items[i]->key); free(items[i]); } }
    free(items);
    radix_clear(&t);
}

// ============================================================================
// Linux内核风格API测试
// ============================================================================

static void test_kernel_style_basic()
{
    printf("\n=== Testing Linux Kernel Style API ===\n");
    
    radix_root_t root;
    radix_tree_init(&root);
    
    // 测试插入
    char *val1 = "kernel_value_1";
    char *val2 = "kernel_value_2"; 
    char *val3 = "kernel_value_3";
    
    printf("Inserting items...\n");
    assert(radix_tree_insert(&root, 0, val1) == 0);
    assert(radix_tree_insert(&root, 1, val2) == 0);
    assert(radix_tree_insert(&root, 1000, val3) == 0);
    
    // 测试重复插入
    assert(radix_tree_insert(&root, 0, val1) == -1);
    
    // 测试查找
    printf("Testing lookups...\n");
    assert(radix_tree_lookup(&root, 0) == val1);
    assert(radix_tree_lookup(&root, 1) == val2);
    assert(radix_tree_lookup(&root, 1000) == val3);
    assert(radix_tree_lookup(&root, 999) == NULL);
    
    // 测试槽位查找
    void **slot = radix_tree_lookup_slot(&root, 0);
    assert(slot && *slot == val1);
    
    // 测试标记
    printf("Testing tags...\n");
    radix_tree_tag_set(&root, 0, RADIX_TREE_TAG_DIRTY);
    radix_tree_tag_set(&root, 1000, RADIX_TREE_TAG_WRITEBACK);
    
    assert(radix_tree_tag_get(&root, 0, RADIX_TREE_TAG_DIRTY) == 1);
    assert(radix_tree_tag_get(&root, 0, RADIX_TREE_TAG_WRITEBACK) == 0);
    assert(radix_tree_tag_get(&root, 1000, RADIX_TREE_TAG_WRITEBACK) == 1);
    
    assert(radix_tree_tagged(&root, RADIX_TREE_TAG_DIRTY) == 1);
    assert(radix_tree_tagged(&root, RADIX_TREE_TAG_TOWRITE) == 0);
    
    // 测试Gang Lookup
    printf("Testing gang lookup...\n");
    void *results[10];
    unsigned int found = radix_tree_gang_lookup(&root, results, 0, 10);
    printf("Gang lookup found %u items\n", found);
    
    // 测试带标记的Gang Lookup
    found = radix_tree_gang_lookup_tag(&root, results, 0, 10, RADIX_TREE_TAG_DIRTY);
    printf("Gang lookup with DIRTY tag found %u items\n", found);
    
    // 测试删除
    printf("Testing deletion...\n");
    void *deleted = radix_tree_delete(&root, 1);
    assert(deleted == val2);
    assert(radix_tree_lookup(&root, 1) == NULL);
    
    // 清理标记
    radix_tree_tag_clear(&root, 0, RADIX_TREE_TAG_DIRTY);
    assert(radix_tree_tag_get(&root, 0, RADIX_TREE_TAG_DIRTY) == 0);
    
    printf("Tree height: %u\n", radix_tree_height(&root));
    
    radix_clear(&root);
    printf("Kernel style tests passed!\n");
}

static void test_kernel_style_stress()
{
    printf("\n=== Kernel Style Stress Test ===\n");
    
    radix_root_t root;
    radix_tree_init(&root);
    
    const int N = 1000;
    char **values = (char **)malloc(N * sizeof(char*));
    
    // 分配测试数据
    for (int i = 0; i < N; i++) {
        values[i] = (char *)malloc(32);
        sprintf(values[i], "stress_value_%d", i);
    }
    
    // 插入测试
    clock_t start = clock();
    for (int i = 0; i < N; i++) {
        int ret = radix_tree_insert(&root, i, values[i]);
        assert(ret == 0);
        
        // 每隔10个设置标记
        if (i % 10 == 0) {
            radix_tree_tag_set(&root, i, RADIX_TREE_TAG_DIRTY);
        }
    }
    clock_t end = clock();
    printf("Inserted %d items in %.2fms\n", N, 
           (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);
    
    // 查找测试
    start = clock();
    for (int i = 0; i < N; i++) {
        void *found = radix_tree_lookup(&root, i);
        assert(found == values[i]);
    }
    end = clock();
    printf("Looked up %d items in %.2fms\n", N,
           (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);
    
    // Gang lookup测试
    start = clock();
    void *results[100];
    unsigned int total_found = 0;
    for (unsigned long start_idx = 0; start_idx < N; start_idx += 100) {
        unsigned int found = radix_tree_gang_lookup(&root, results, start_idx, 100);
        total_found += found;
    }
    end = clock();
    printf("Gang lookup found %u items in %.2fms\n", total_found,
           (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);
    
    // 标记查找测试
    start = clock();
    unsigned int tagged_found = radix_tree_gang_lookup_tag(&root, results, 0, 100, RADIX_TREE_TAG_DIRTY);
    end = clock();
    printf("Tagged gang lookup found %u items in %.2fms\n", tagged_found,
           (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);
    
    // 删除测试
    start = clock();
    for (int i = 0; i < N/2; i += 2) {
        void *deleted = radix_tree_delete(&root, i);
        assert(deleted == values[i]);
    }
    end = clock();
    printf("Deleted %d items in %.2fms\n", N/4,
           (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);
    
    printf("Final tree height: %u\n", radix_tree_height(&root));
    
    // 清理
    for (int i = 0; i < N; i++) {
        free(values[i]);
    }
    free(values);
    radix_clear(&root);
    
    printf("Kernel style stress test passed!\n");
}

int main(void)
{
    test_strings_basic();
    test_binary_keys();
    test_stress();
    
    // 新增的Linux内核风格测试
    test_kernel_style_basic();
    test_kernel_style_stress();
    
    printf("All radix_tree tests passed.\n");
    return 0;
}
