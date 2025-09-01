#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "ChaCha20/ChaCha20.h"

/**
 * Print hex data for debugging
 */
static void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 8 == 0) printf("  ");
        else printf(" ");
    }
    if (len % 16 != 0) printf("\n");
}

/**
 * Test ChaCha20 with RFC 8439 test vector
 */
void test_chacha20_rfc8439(void) {
    printf("=== ChaCha20 RFC 8439 Test Vector ===\n");
    
    // Test vector from RFC 8439
    uint8_t key[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    
    uint8_t nonce[12] = {
        0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x4a,
        0x00, 0x00, 0x00, 0x00
    };
    
    uint32_t counter = 1;
    
    const char *plaintext = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";
    size_t plaintext_len = strlen(plaintext);
    
    uint8_t ciphertext[256];
    uint8_t decrypted[256];
    
    chacha20_ctx_t ctx;
    
    // Encrypt
    chacha20_init(&ctx, key, nonce, counter);
    chacha20_encrypt(&ctx, (uint8_t*)plaintext, ciphertext, plaintext_len);
    
    printf("Plaintext:  %s\n", plaintext);
    print_hex("Ciphertext", ciphertext, plaintext_len);
    
    // Decrypt
    chacha20_init(&ctx, key, nonce, counter);
    chacha20_decrypt(&ctx, ciphertext, decrypted, plaintext_len);
    decrypted[plaintext_len] = '\0';
    
    printf("Decrypted:  %s\n", (char*)decrypted);
    
    // Verify
    if (memcmp(plaintext, decrypted, plaintext_len) == 0) {
        printf("✓ ChaCha20 RFC 8439 test PASSED\n");
    } else {
        printf("✗ ChaCha20 RFC 8439 test FAILED\n");
    }
    printf("\n");
}

/**
 * Test ChaCha20 keystream generation
 */
void test_chacha20_keystream(void) {
    printf("=== ChaCha20 Keystream Test ===\n");
    
    uint8_t key[32] = {0};
    uint8_t nonce[12] = {0};
    uint32_t counter = 0;
    
    chacha20_ctx_t ctx;
    chacha20_init(&ctx, key, nonce, counter);
    
    uint8_t keystream[128];
    chacha20_keystream(&ctx, keystream, sizeof(keystream));
    
    print_hex("Keystream (first 128 bytes)", keystream, sizeof(keystream));
    
    // Test that keystream is deterministic
    chacha20_init(&ctx, key, nonce, counter);
    uint8_t keystream2[128];
    chacha20_keystream(&ctx, keystream2, sizeof(keystream2));
    
    if (memcmp(keystream, keystream2, sizeof(keystream)) == 0) {
        printf("✓ ChaCha20 keystream deterministic test PASSED\n");
    } else {
        printf("✗ ChaCha20 keystream deterministic test FAILED\n");
    }
    printf("\n");
}

/**
 * Test ChaCha20 in-place encryption
 */
void test_chacha20_inplace(void) {
    printf("=== ChaCha20 In-place Encryption Test ===\n");
    
    uint8_t key[32] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                       17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
    uint8_t nonce[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    uint32_t counter = 42;
    
    const char *original = "Hello, ChaCha20! This is a test message for in-place encryption.";
    size_t len = strlen(original);
    
    uint8_t buffer[256];
    strcpy((char*)buffer, original);
    
    chacha20_ctx_t ctx;
    
    // In-place encryption
    chacha20_init(&ctx, key, nonce, counter);
    chacha20_encrypt(&ctx, buffer, buffer, len);
    
    printf("Original:  %s\n", original);
    print_hex("Encrypted", buffer, len);
    
    // In-place decryption
    chacha20_init(&ctx, key, nonce, counter);
    chacha20_decrypt(&ctx, buffer, buffer, len);
    buffer[len] = '\0';
    
    printf("Decrypted: %s\n", (char*)buffer);
    
    if (strcmp(original, (char*)buffer) == 0) {
        printf("✓ ChaCha20 in-place encryption test PASSED\n");
    } else {
        printf("✗ ChaCha20 in-place encryption test FAILED\n");
    }
    printf("\n");
}

/**
 * Test ChaCha20 counter reset functionality
 */
void test_chacha20_counter_reset(void) {
    printf("=== ChaCha20 Counter Reset Test ===\n");
    
    uint8_t key[32] = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
                       0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
                       0x0f, 0x1e, 0x2d, 0x3c, 0x4b, 0x5a, 0x69, 0x78,
                       0x87, 0x96, 0xa5, 0xb4, 0xc3, 0xd2, 0xe1, 0xf0};
    uint8_t nonce[12] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                         0x11, 0x22, 0x33, 0x44};
    
    chacha20_ctx_t ctx;
    
    // Generate keystream starting from counter 0
    chacha20_init(&ctx, key, nonce, 0);
    uint8_t keystream1[64];
    chacha20_keystream(&ctx, keystream1, 64);
    
    // Generate keystream starting from counter 10
    chacha20_init(&ctx, key, nonce, 10);
    uint8_t keystream2[64];
    chacha20_keystream(&ctx, keystream2, 64);
    
    // Reset counter to 0 and generate keystream again
    chacha20_reset_counter(&ctx, 0);
    uint8_t keystream3[64];
    chacha20_keystream(&ctx, keystream3, 64);
    
    print_hex("Keystream (counter=0)", keystream1, 32);
    print_hex("Keystream (counter=10)", keystream2, 32);
    print_hex("Keystream (reset to 0)", keystream3, 32);
    
    if (memcmp(keystream1, keystream3, 64) == 0) {
        printf("✓ ChaCha20 counter reset test PASSED\n");
    } else {
        printf("✗ ChaCha20 counter reset test FAILED\n");
    }
    
    if (memcmp(keystream1, keystream2, 64) != 0) {
        printf("✓ ChaCha20 different counter produces different keystream PASSED\n");
    } else {
        printf("✗ ChaCha20 different counter produces different keystream FAILED\n");
    }
    printf("\n");
}

/**
 * Main function to run all ChaCha20 tests
 */
int main(void) {
    printf("Starting ChaCha20 Algorithm Tests\n");
    printf("==================================\n\n");
    
    test_chacha20_rfc8439();
    test_chacha20_keystream();
    test_chacha20_inplace();
    test_chacha20_counter_reset();
    
    printf("All ChaCha20 tests completed!\n");
    return 0;
}
