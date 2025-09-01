#include "md5.h"
#include <string.h>
#include <stdio.h>

// 左旋转函数
#define ROTLEFT(value, amount) (((value) << (amount)) | ((value) >> (32 - (amount))))

// MD5 辅助函数
#define F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | ~(z)))

// MD5 轮函数
#define FF(a, b, c, d, x, s, ac) { \
    (a) += F((b), (c), (d)) + (x) + (ac); \
    (a) = ROTLEFT((a), (s)); \
    (a) += (b); \
}

#define GG(a, b, c, d, x, s, ac) { \
    (a) += G((b), (c), (d)) + (x) + (ac); \
    (a) = ROTLEFT((a), (s)); \
    (a) += (b); \
}

#define HH(a, b, c, d, x, s, ac) { \
    (a) += H((b), (c), (d)) + (x) + (ac); \
    (a) = ROTLEFT((a), (s)); \
    (a) += (b); \
}

#define II(a, b, c, d, x, s, ac) { \
    (a) += I((b), (c), (d)) + (x) + (ac); \
    (a) = ROTLEFT((a), (s)); \
    (a) += (b); \
}

// 将字节数组转换为32位无符号整数（小端序）
static uint32_t bytes_to_uint32(const uint8_t *bytes) {
    return (uint32_t)bytes[0] | 
           ((uint32_t)bytes[1] << 8) | 
           ((uint32_t)bytes[2] << 16) | 
           ((uint32_t)bytes[3] << 24);
}

// 将32位无符号整数转换为字节数组（小端序）
static void uint32_to_bytes(uint32_t value, uint8_t *bytes) {
    bytes[0] = (uint8_t)(value & 0xFF);
    bytes[1] = (uint8_t)((value >> 8) & 0xFF);
    bytes[2] = (uint8_t)((value >> 16) & 0xFF);
    bytes[3] = (uint8_t)((value >> 24) & 0xFF);
}

// MD5 核心变换函数（处理512位块）
static void md5_transform(uint32_t state[4], const uint8_t block[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t x[16];
    
    // 将64字节块转换为16个32位字
    for (int i = 0; i < 16; i++) {
        x[i] = bytes_to_uint32(&block[i * 4]);
    }
    
    // 第一轮 (Round 1)
    FF(a, b, c, d, x[ 0],  7, 0xd76aa478);
    FF(d, a, b, c, x[ 1], 12, 0xe8c7b756);
    FF(c, d, a, b, x[ 2], 17, 0x242070db);
    FF(b, c, d, a, x[ 3], 22, 0xc1bdceee);
    FF(a, b, c, d, x[ 4],  7, 0xf57c0faf);
    FF(d, a, b, c, x[ 5], 12, 0x4787c62a);
    FF(c, d, a, b, x[ 6], 17, 0xa8304613);
    FF(b, c, d, a, x[ 7], 22, 0xfd469501);
    FF(a, b, c, d, x[ 8],  7, 0x698098d8);
    FF(d, a, b, c, x[ 9], 12, 0x8b44f7af);
    FF(c, d, a, b, x[10], 17, 0xffff5bb1);
    FF(b, c, d, a, x[11], 22, 0x895cd7be);
    FF(a, b, c, d, x[12],  7, 0x6b901122);
    FF(d, a, b, c, x[13], 12, 0xfd987193);
    FF(c, d, a, b, x[14], 17, 0xa679438e);
    FF(b, c, d, a, x[15], 22, 0x49b40821);
    
    // 第二轮 (Round 2)
    GG(a, b, c, d, x[ 1],  5, 0xf61e2562);
    GG(d, a, b, c, x[ 6],  9, 0xc040b340);
    GG(c, d, a, b, x[11], 14, 0x265e5a51);
    GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aa);
    GG(a, b, c, d, x[ 5],  5, 0xd62f105d);
    GG(d, a, b, c, x[10],  9, 0x02441453);
    GG(c, d, a, b, x[15], 14, 0xd8a1e681);
    GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8);
    GG(a, b, c, d, x[ 9],  5, 0x21e1cde6);
    GG(d, a, b, c, x[14],  9, 0xc33707d6);
    GG(c, d, a, b, x[ 3], 14, 0xf4d50d87);
    GG(b, c, d, a, x[ 8], 20, 0x455a14ed);
    GG(a, b, c, d, x[13],  5, 0xa9e3e905);
    GG(d, a, b, c, x[ 2],  9, 0xfcefa3f8);
    GG(c, d, a, b, x[ 7], 14, 0x676f02d9);
    GG(b, c, d, a, x[12], 20, 0x8d2a4c8a);
    
    // 第三轮 (Round 3)
    HH(a, b, c, d, x[ 5],  4, 0xfffa3942);
    HH(d, a, b, c, x[ 8], 11, 0x8771f681);
    HH(c, d, a, b, x[11], 16, 0x6d9d6122);
    HH(b, c, d, a, x[14], 23, 0xfde5380c);
    HH(a, b, c, d, x[ 1],  4, 0xa4beea44);
    HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9);
    HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60);
    HH(b, c, d, a, x[10], 23, 0xbebfbc70);
    HH(a, b, c, d, x[13],  4, 0x289b7ec6);
    HH(d, a, b, c, x[ 0], 11, 0xeaa127fa);
    HH(c, d, a, b, x[ 3], 16, 0xd4ef3085);
    HH(b, c, d, a, x[ 6], 23, 0x04881d05);
    HH(a, b, c, d, x[ 9],  4, 0xd9d4d039);
    HH(d, a, b, c, x[12], 11, 0xe6db99e5);
    HH(c, d, a, b, x[15], 16, 0x1fa27cf8);
    HH(b, c, d, a, x[ 2], 23, 0xc4ac5665);
    
    // 第四轮 (Round 4)
    II(a, b, c, d, x[ 0],  6, 0xf4292244);
    II(d, a, b, c, x[ 7], 10, 0x432aff97);
    II(c, d, a, b, x[14], 15, 0xab9423a7);
    II(b, c, d, a, x[ 5], 21, 0xfc93a039);
    II(a, b, c, d, x[12],  6, 0x655b59c3);
    II(d, a, b, c, x[ 3], 10, 0x8f0ccc92);
    II(c, d, a, b, x[10], 15, 0xffeff47d);
    II(b, c, d, a, x[ 1], 21, 0x85845dd1);
    II(a, b, c, d, x[ 8],  6, 0x6fa87e4f);
    II(d, a, b, c, x[15], 10, 0xfe2ce6e0);
    II(c, d, a, b, x[ 6], 15, 0xa3014314);
    II(b, c, d, a, x[13], 21, 0x4e0811a1);
    II(a, b, c, d, x[ 4],  6, 0xf7537e82);
    II(d, a, b, c, x[11], 10, 0xbd3af235);
    II(c, d, a, b, x[ 2], 15, 0x2ad7d2bb);
    II(b, c, d, a, x[ 9], 21, 0xeb86d391);
    
    // 累加结果
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

void md5_init(md5_context_t *ctx) {
    if (!ctx) return;
    
    // 初始化状态变量 (RFC 1321)
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
    
    // 初始化位计数器
    ctx->count[0] = 0;
    ctx->count[1] = 0;
}

void md5_update(md5_context_t *ctx, const uint8_t *data, size_t len) {
    if (!ctx || !data) return;
    
    size_t index = (ctx->count[0] >> 3) & 0x3F;  // 当前缓冲区的字节索引
    size_t part_len = 64 - index;               // 缓冲区剩余空间
    
    // 更新位计数器
    ctx->count[0] += (uint32_t)(len << 3);
    if (ctx->count[0] < (uint32_t)(len << 3)) {
        ctx->count[1]++;
    }
    ctx->count[1] += (uint32_t)(len >> 29);
    
    size_t i = 0;
    
    // 如果输入数据能填满当前缓冲区
    if (len >= part_len) {
        memcpy(&ctx->buffer[index], data, part_len);
        md5_transform(ctx->state, ctx->buffer);
        
        // 处理完整的64字节块
        for (i = part_len; i + 63 < len; i += 64) {
            md5_transform(ctx->state, &data[i]);
        }
        
        index = 0;
    }
    
    // 保存剩余数据到缓冲区
    memcpy(&ctx->buffer[index], &data[i], len - i);
}

void md5_final(md5_context_t *ctx, uint8_t digest[MD5_DIGEST_LENGTH]) {
    if (!ctx || !digest) return;
    
    uint8_t bits[8];
    size_t index = (ctx->count[0] >> 3) & 0x3F;
    size_t pad_len = (index < 56) ? (56 - index) : (120 - index);
    
    // 将位长度转换为字节数组（小端序）
    uint32_to_bytes(ctx->count[0], &bits[0]);
    uint32_to_bytes(ctx->count[1], &bits[4]);
    
    // 添加填充（1位后跟0位）
    uint8_t padding[64];
    padding[0] = 0x80;
    memset(&padding[1], 0, 63);
    
    md5_update(ctx, padding, pad_len);
    
    // 添加原始长度（以位为单位）
    md5_update(ctx, bits, 8);
    
    // 输出最终哈希值（小端序）
    for (int i = 0; i < 4; i++) {
        uint32_to_bytes(ctx->state[i], &digest[i * 4]);
    }
}

void md5_hash(const uint8_t *data, size_t len, uint8_t digest[MD5_DIGEST_LENGTH]) {
    md5_context_t ctx;
    md5_init(&ctx);
    md5_update(&ctx, data, len);
    md5_final(&ctx, digest);
}

void md5_digest_to_hex(const uint8_t digest[MD5_DIGEST_LENGTH], char hex_str[33]) {
    if (!digest || !hex_str) return;
    
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(&hex_str[i * 2], "%02x", digest[i]);
    }
    hex_str[32] = '\0';
}
