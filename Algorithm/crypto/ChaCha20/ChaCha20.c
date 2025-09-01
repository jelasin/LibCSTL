#include "ChaCha20.h"
#include <string.h>

// ChaCha20 constants
static const uint32_t CHACHA20_CONSTANTS[4] = {
    0x61707865, 0x3320646e, 0x79622d32, 0x6b206574
};

/**
 * Little-endian 32-bit word read
 */
static uint32_t read_le32(const uint8_t *p) {
    return ((uint32_t)p[0]) |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

/**
 * Little-endian 32-bit word write
 */
static void write_le32(uint8_t *p, uint32_t val) {
    p[0] = (uint8_t)(val);
    p[1] = (uint8_t)(val >> 8);
    p[2] = (uint8_t)(val >> 16);
    p[3] = (uint8_t)(val >> 24);
}

/**
 * ChaCha20 quarter round operation
 */
static void chacha20_quarter_round(uint32_t *x, int a, int b, int c, int d) {
    x[a] += x[b]; x[d] ^= x[a]; x[d] = (x[d] << 16) | (x[d] >> 16);
    x[c] += x[d]; x[b] ^= x[c]; x[b] = (x[b] << 12) | (x[b] >> 20);
    x[a] += x[b]; x[d] ^= x[a]; x[d] = (x[d] << 8) | (x[d] >> 24);
    x[c] += x[d]; x[b] ^= x[c]; x[b] = (x[b] << 7) | (x[b] >> 25);
}

/**
 * ChaCha20 block function
 */
static void chacha20_block(const uint32_t input[16], uint8_t output[64]) {
    uint32_t x[16];
    int i;
    
    // Copy input to working state
    for (i = 0; i < 16; i++) {
        x[i] = input[i];
    }
    
    // 20 rounds (10 double rounds)
    for (i = 0; i < 10; i++) {
        // Column rounds
        chacha20_quarter_round(x, 0, 4, 8, 12);
        chacha20_quarter_round(x, 1, 5, 9, 13);
        chacha20_quarter_round(x, 2, 6, 10, 14);
        chacha20_quarter_round(x, 3, 7, 11, 15);
        
        // Diagonal rounds
        chacha20_quarter_round(x, 0, 5, 10, 15);
        chacha20_quarter_round(x, 1, 6, 11, 12);
        chacha20_quarter_round(x, 2, 7, 8, 13);
        chacha20_quarter_round(x, 3, 4, 9, 14);
    }
    
    // Add input to output
    for (i = 0; i < 16; i++) {
        x[i] += input[i];
        write_le32(output + 4 * i, x[i]);
    }
}

void chacha20_init(chacha20_ctx_t *ctx, const uint8_t key[32], const uint8_t nonce[12], uint32_t counter) {
    int i;
    
    // Constants "expand 32-byte k"
    ctx->state[0] = CHACHA20_CONSTANTS[0];
    ctx->state[1] = CHACHA20_CONSTANTS[1];
    ctx->state[2] = CHACHA20_CONSTANTS[2];
    ctx->state[3] = CHACHA20_CONSTANTS[3];
    
    // 32-byte key
    for (i = 0; i < 8; i++) {
        ctx->state[4 + i] = read_le32(key + 4 * i);
    }
    
    // Counter
    ctx->state[12] = counter;
    
    // 12-byte nonce
    for (i = 0; i < 3; i++) {
        ctx->state[13 + i] = read_le32(nonce + 4 * i);
    }
    
    // Initialize keystream position
    ctx->counter = counter;
    ctx->keystream_pos = 64; // Force generation of new keystream block
}

static void chacha20_generate_keystream(chacha20_ctx_t *ctx) {
    ctx->state[12] = ctx->counter;
    chacha20_block(ctx->state, ctx->keystream);
    ctx->counter++;
    ctx->keystream_pos = 0;
}

void chacha20_encrypt(chacha20_ctx_t *ctx, const uint8_t *input, uint8_t *output, size_t length) {
    size_t i;
    
    for (i = 0; i < length; i++) {
        if (ctx->keystream_pos >= 64) {
            chacha20_generate_keystream(ctx);
        }
        
        output[i] = input[i] ^ ctx->keystream[ctx->keystream_pos];
        ctx->keystream_pos++;
    }
}

void chacha20_decrypt(chacha20_ctx_t *ctx, const uint8_t *input, uint8_t *output, size_t length) {
    // ChaCha20 is a stream cipher, so decryption is the same as encryption
    chacha20_encrypt(ctx, input, output, length);
}

void chacha20_keystream(chacha20_ctx_t *ctx, uint8_t *output, size_t length) {
    size_t i;
    
    for (i = 0; i < length; i++) {
        if (ctx->keystream_pos >= 64) {
            chacha20_generate_keystream(ctx);
        }
        
        output[i] = ctx->keystream[ctx->keystream_pos];
        ctx->keystream_pos++;
    }
}

void chacha20_reset_counter(chacha20_ctx_t *ctx, uint32_t counter) {
    ctx->counter = counter;
    ctx->keystream_pos = 64; // Force generation of new keystream block
}
