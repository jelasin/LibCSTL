/**
 * @file example.c
 * @brief 演示 Algorithm/hash 目录下所有哈希算法的示例程序
 * @author LibCSTL
 * @date 2025-09-01
 * 
 * 本文件演示以下哈希算法的使用：
 * 1. 字符串哈希算法：AP, BKDR, DJB2, ELF, JS, PJW, RS, SDBM
 * 2. 简单哈希算法：Division Hash, Multiplication Hash
 * 3. 密码学哈希算法：MD5
 * 
 * gcc example.c .\*\*.c -o test
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 包含所有哈希算法头文件
#include "APHash/APHash.h"
#include "BKDRHash/BKDRHash.h"
#include "DJB2Hash/DJB2Hash.h"
#include "ELFHash/ELFHash.h"
#include "JSHash/JSHash.h"
#include "PJWHash/PJWHash.h"
#include "RSHash/RSHash.h"
#include "SDBMHash/SDBMHash.h"
#include "SimpleHash/SimpleHash.h"
#include "MD5/md5.h"

// 测试用的字符串
static const char* test_strings[] = {
    "hello",
    "world",
    "LibCSTL",
    "Hash Algorithm",
    "The quick brown fox jumps over the lazy dog",
    "1234567890",
    "abcdefghijklmnopqrstuvwxyz",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    "!@#$%^&*()_+-=[]{}|;':\",./<>?",
    ""  // 空字符串
};

#define TEST_STRING_COUNT (sizeof(test_strings) / sizeof(test_strings[0]))

/**
 * @brief 演示字符串哈希算法
 */
void demo_string_hash_algorithms(void) {
    printf("=== 字符串哈希算法演示 ===\n\n");
    
    // 表头
    printf("%-30s | %-10s | %-10s | %-10s | %-10s | %-10s | %-10s | %-10s | %-10s\n",
           "字符串", "AP", "BKDR", "DJB2", "ELF", "JS", "PJW", "RS", "SDBM");
    printf("-------------------------------");
    for (int i = 0; i < 8; i++) {
        printf("+-----------");
    }
    printf("\n");
    
    // 对每个测试字符串计算各种哈希值
    for (size_t i = 0; i < TEST_STRING_COUNT; i++) {
        const char* str = test_strings[i];
        const char* display_str = (strlen(str) == 0) ? "(空字符串)" : str;
        
        printf("%-30s | 0x%08X | 0x%08X | 0x%08X | 0x%08X | 0x%08X | 0x%08X | 0x%08X | 0x%08X\n",
               display_str,
               APHash(str),
               BKDRHash(str),
               DJB2Hash(str),
               ELFHash(str),
               JSHash(str),
               PJWHash(str),
               RSHash(str),
               SDBMHash(str));
    }
    printf("\n");
}

/**
 * @brief 演示简单哈希算法（除法散列和乘法散列）
 */
void demo_simple_hash_algorithms(void) {
    printf("=== 简单哈希算法演示 ===\n\n");
    
    // 设置哈希表大小
    unsigned int table_sizes[] = {101, 503, 1009, 2003};
    size_t table_count = sizeof(table_sizes) / sizeof(table_sizes[0]);
    
    for (size_t t = 0; t < table_count; t++) {
        unsigned int table_size = table_sizes[t];
        set_tableSize(table_size);
        
        printf("哈希表大小: %u\n", table_size);
        printf("%-30s | %-15s | %-15s\n", "字符串", "除法散列", "乘法散列");
        printf("-------------------------------+----------------+----------------\n");
        
        for (size_t i = 0; i < TEST_STRING_COUNT; i++) {
            const char* str = test_strings[i];
            const char* display_str = (strlen(str) == 0) ? "(空字符串)" : str;
            
            printf("%-30s | %-15u | %-15u\n",
                   display_str,
                   DivisionHash(str),
                   MultiplicationHash(str));
        }
        printf("\n");
    }
}

/**
 * @brief 演示MD5哈希算法
 */
void demo_md5_algorithm(void) {
    printf("=== MD5 哈希算法演示 ===\n\n");
    
    printf("%-30s | %-34s\n", "字符串", "MD5摘要");
    printf("-------------------------------+----------------------------------\n");
    
    for (size_t i = 0; i < TEST_STRING_COUNT; i++) {
        const char* str = test_strings[i];
        const char* display_str = (strlen(str) == 0) ? "(空字符串)" : str;
        
        uint8_t digest[MD5_DIGEST_LENGTH];
        char hex_str[33];
        
        // 计算MD5哈希值
        md5_hash((const uint8_t*)str, strlen(str), digest);
        
        // 转换为十六进制字符串
        md5_digest_to_hex(digest, hex_str);
        
        printf("%-30s | %s\n", display_str, hex_str);
    }
    printf("\n");
}

/**
 * @brief 演示MD5算法的分块处理功能
 */
void demo_md5_incremental(void) {
    printf("=== MD5 增量计算演示 ===\n\n");
    
    const char* test_data = "The quick brown fox jumps over the lazy dog";
    printf("完整字符串: \"%s\"\n", test_data);
    
    // 一次性计算
    uint8_t digest1[MD5_DIGEST_LENGTH];
    char hex_str1[33];
    md5_hash((const uint8_t*)test_data, strlen(test_data), digest1);
    md5_digest_to_hex(digest1, hex_str1);
    printf("一次性计算MD5: %s\n", hex_str1);
    
    // 分块计算
    md5_context_t ctx;
    uint8_t digest2[MD5_DIGEST_LENGTH];
    char hex_str2[33];
    
    md5_init(&ctx);
    
    // 分三块处理
    md5_update(&ctx, (const uint8_t*)"The quick brown fox ", 20);
    md5_update(&ctx, (const uint8_t*)"jumps over ", 11);
    md5_update(&ctx, (const uint8_t*)"the lazy dog", 12);
    
    md5_final(&ctx, digest2);
    md5_digest_to_hex(digest2, hex_str2);
    printf("分块计算MD5:   %s\n", hex_str2);
    
    // 验证结果是否一致
    if (memcmp(digest1, digest2, MD5_DIGEST_LENGTH) == 0) {
        printf("✓ 两种计算方式结果一致！\n");
    } else {
        printf("✗ 两种计算方式结果不一致！\n");
    }
    printf("\n");
}

/**
 * @brief 演示哈希冲突检测
 */
void demo_hash_collision_detection(void) {
    printf("=== 哈希冲突检测演示 ===\n\n");
    
    // 使用一个较小的哈希表来增加冲突概率
    set_tableSize(7);  // 使用质数7作为表大小
    
    printf("使用小哈希表(大小=7)检测冲突:\n");
    printf("%-20s | %-10s | %-10s | %-10s\n", "字符串", "AP Hash", "除法散列", "乘法散列");
    printf("---------------------+-----------+-----------+-----------\n");
    
    // 记录冲突
    unsigned int ap_conflicts = 0, div_conflicts = 0, mult_conflicts = 0;
    unsigned int ap_used[7] = {0}, div_used[7] = {0}, mult_used[7] = {0};
    
    for (size_t i = 0; i < TEST_STRING_COUNT; i++) {
        const char* str = test_strings[i];
        
        unsigned int ap_hash = APHash(str) % 7;
        unsigned int div_hash = DivisionHash(str);
        unsigned int mult_hash = MultiplicationHash(str);
        
        printf("%-20s | %10u | %10u | %10u", str, ap_hash, div_hash, mult_hash);
        
        // 检测冲突
        if (ap_used[ap_hash]) {
            printf(" [AP冲突]");
            ap_conflicts++;
        } else {
            ap_used[ap_hash] = 1;
        }
        
        if (div_used[div_hash]) {
            printf(" [Div冲突]");
            div_conflicts++;
        } else {
            div_used[div_hash] = 1;
        }
        
        if (mult_used[mult_hash]) {
            printf(" [Mult冲突]");
            mult_conflicts++;
        } else {
            mult_used[mult_hash] = 1;
        }
        
        printf("\n");
    }
    
    printf("\n冲突统计:\n");
    printf("AP Hash 冲突次数: %u\n", ap_conflicts);
    printf("除法散列 冲突次数: %u\n", div_conflicts);
    printf("乘法散列 冲突次数: %u\n", mult_conflicts);
    printf("\n");
}

/**
 * @brief 主函数
 */
int main(void) {
    printf("LibCSTL Hash Algorithm Demo\n");
    printf("===========================\n\n");
    
    // 演示各种哈希算法
    demo_string_hash_algorithms();
    demo_simple_hash_algorithms();
    demo_md5_algorithm();
    demo_md5_incremental();
    demo_hash_collision_detection();
    
    printf("演示完成！\n");

    return 0;
}
