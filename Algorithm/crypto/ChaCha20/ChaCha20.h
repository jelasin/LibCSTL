#ifndef CHACHA20_H
#define CHACHA20_H

#include <stdint.h>
#include <stddef.h>

/**
 * ChaCha20 encryption/decryption context
 */
typedef struct {
    uint32_t state[16];     // ChaCha20 state
    uint32_t counter;       // Block counter
    uint8_t keystream[64];  // Generated keystream block
    size_t keystream_pos;   // Position in current keystream block
} chacha20_ctx_t;

/**
 * Initialize ChaCha20 context with key, nonce and counter
 * @param ctx ChaCha20 context to initialize
 * @param key 32-byte key
 * @param nonce 12-byte nonce
 * @param counter Initial counter value (usually 0 or 1)
 */
void chacha20_init(chacha20_ctx_t *ctx, const uint8_t key[32], const uint8_t nonce[12], uint32_t counter);

/**
 * Encrypt/decrypt data using ChaCha20
 * @param ctx ChaCha20 context
 * @param input Input data
 * @param output Output buffer (can be the same as input for in-place operation)
 * @param length Length of data in bytes
 */
void chacha20_encrypt(chacha20_ctx_t *ctx, const uint8_t *input, uint8_t *output, size_t length);

/**
 * Decrypt data using ChaCha20 (same as encrypt due to stream cipher nature)
 * @param ctx ChaCha20 context
 * @param input Input data
 * @param output Output buffer (can be the same as input for in-place operation)
 * @param length Length of data in bytes
 */
void chacha20_decrypt(chacha20_ctx_t *ctx, const uint8_t *input, uint8_t *output, size_t length);

/**
 * Generate raw keystream without encryption/decryption
 * @param ctx ChaCha20 context
 * @param output Output buffer for keystream
 * @param length Length of keystream to generate
 */
void chacha20_keystream(chacha20_ctx_t *ctx, uint8_t *output, size_t length);

/**
 * Reset the ChaCha20 context to initial state with new counter
 * @param ctx ChaCha20 context
 * @param counter New counter value
 */
void chacha20_reset_counter(chacha20_ctx_t *ctx, uint32_t counter);

#endif // CHACHA20_H
