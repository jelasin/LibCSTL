#ifndef MD5_H
#define MD5_H

#include <stdint.h>
#include <stddef.h>

/**
 * MD5 算法实现
 * RFC 1321 标准
 */

// MD5 上下文结构体
typedef struct {
    uint32_t state[4];          // 状态变量 A, B, C, D
    uint32_t count[2];          // 位计数器，低32位和高32位
    uint8_t buffer[64];         // 输入缓冲区
} md5_context_t;

// MD5 摘要长度（字节）
#define MD5_DIGEST_LENGTH 16

/**
 * 初始化MD5上下文
 * @param ctx MD5上下文指针
 */
void md5_init(md5_context_t *ctx);

/**
 * 更新MD5哈希值（处理输入数据）
 * @param ctx MD5上下文指针
 * @param data 输入数据指针
 * @param len 输入数据长度
 */
void md5_update(md5_context_t *ctx, const uint8_t *data, size_t len);

/**
 * 完成MD5计算并输出最终哈希值
 * @param ctx MD5上下文指针
 * @param digest 输出的MD5摘要（16字节）
 */
void md5_final(md5_context_t *ctx, uint8_t digest[MD5_DIGEST_LENGTH]);

/**
 * 一次性计算MD5哈希值（便捷函数）
 * @param data 输入数据指针
 * @param len 输入数据长度
 * @param digest 输出的MD5摘要（16字节）
 */
void md5_hash(const uint8_t *data, size_t len, uint8_t digest[MD5_DIGEST_LENGTH]);

/**
 * 将MD5摘要转换为十六进制字符串
 * @param digest MD5摘要（16字节）
 * @param hex_str 输出的十六进制字符串（至少33字节，包含'\0'）
 */
void md5_digest_to_hex(const uint8_t digest[MD5_DIGEST_LENGTH], char hex_str[33]);

#endif // MD5_H
