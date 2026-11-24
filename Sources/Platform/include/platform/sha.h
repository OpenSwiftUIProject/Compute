#pragma once

#if __APPLE__

#include <CommonCrypto/CommonDigest.h>

typedef CC_SHA1_CTX PLATFORM_SHA1_CTX;

#define PLATFORM_SHA1_Init CC_SHA1_Init
#define PLATFORM_SHA1_Update CC_SHA1_Update
#define PLATFORM_SHA1_Final CC_SHA1_Final
#define PLATFORM_SHA1_DIGEST_LENGTH CC_SHA1_DIGEST_LENGTH

#elif __ANDROID__

#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    uint32_t state[5];
    uint64_t bit_count;
    uint8_t buffer[64];
    size_t buffer_count;
} PLATFORM_SHA1_CTX;

#define PLATFORM_SHA1_DIGEST_LENGTH 20

static inline uint32_t platform_sha1_rotate_left(uint32_t value, uint32_t bits) {
    return (value << bits) | (value >> (32 - bits));
}

static inline uint32_t platform_sha1_load_be32(const uint8_t *bytes) {
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) |
           (uint32_t)bytes[3];
}

static inline void platform_sha1_store_be32(uint8_t *bytes, uint32_t value) {
    bytes[0] = (uint8_t)(value >> 24);
    bytes[1] = (uint8_t)(value >> 16);
    bytes[2] = (uint8_t)(value >> 8);
    bytes[3] = (uint8_t)value;
}

static inline void platform_sha1_store_be64(uint8_t *bytes, uint64_t value) {
    for (int i = 7; i >= 0; --i) {
        bytes[7 - i] = (uint8_t)(value >> (i * 8));
    }
}

static inline void platform_sha1_transform(PLATFORM_SHA1_CTX *context, const uint8_t block[64]) {
    uint32_t words[80];
    for (int i = 0; i < 16; ++i) {
        words[i] = platform_sha1_load_be32(block + i * 4);
    }
    for (int i = 16; i < 80; ++i) {
        words[i] = platform_sha1_rotate_left(words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16], 1);
    }

    uint32_t a = context->state[0];
    uint32_t b = context->state[1];
    uint32_t c = context->state[2];
    uint32_t d = context->state[3];
    uint32_t e = context->state[4];

    for (int i = 0; i < 80; ++i) {
        uint32_t f;
        uint32_t k;
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }
        uint32_t temp = platform_sha1_rotate_left(a, 5) + f + e + k + words[i];
        e = d;
        d = c;
        c = platform_sha1_rotate_left(b, 30);
        b = a;
        a = temp;
    }

    context->state[0] += a;
    context->state[1] += b;
    context->state[2] += c;
    context->state[3] += d;
    context->state[4] += e;
}

static inline int platform_sha1_init(PLATFORM_SHA1_CTX *context) {
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->bit_count = 0;
    context->buffer_count = 0;
    return 1;
}

static inline int platform_sha1_update(PLATFORM_SHA1_CTX *context, const void *data, size_t length) {
    const uint8_t *bytes = (const uint8_t *)data;
    context->bit_count += (uint64_t)length * 8;

    while (length > 0) {
        size_t available = sizeof(context->buffer) - context->buffer_count;
        size_t count = length < available ? length : available;
        memcpy(context->buffer + context->buffer_count, bytes, count);
        context->buffer_count += count;
        bytes += count;
        length -= count;

        if (context->buffer_count == sizeof(context->buffer)) {
            platform_sha1_transform(context, context->buffer);
            context->buffer_count = 0;
        }
    }
    return 1;
}

static inline int platform_sha1_final(uint8_t digest[PLATFORM_SHA1_DIGEST_LENGTH], PLATFORM_SHA1_CTX *context) {
    uint8_t marker = 0x80;
    uint8_t zero = 0;
    uint8_t length_bytes[8];
    platform_sha1_store_be64(length_bytes, context->bit_count);

    platform_sha1_update(context, &marker, 1);
    while (context->buffer_count != 56) {
        platform_sha1_update(context, &zero, 1);
    }
    platform_sha1_update(context, length_bytes, sizeof(length_bytes));

    for (int i = 0; i < 5; ++i) {
        platform_sha1_store_be32(digest + i * 4, context->state[i]);
    }
    return 1;
}

#define PLATFORM_SHA1_Init platform_sha1_init
#define PLATFORM_SHA1_Update platform_sha1_update
#define PLATFORM_SHA1_Final platform_sha1_final

#else

#include <openssl/sha.h>

typedef SHA_CTX PLATFORM_SHA1_CTX;

#define PLATFORM_SHA1_Init SHA1_Init
#define PLATFORM_SHA1_Update SHA1_Update
#define PLATFORM_SHA1_Final SHA1_Final
#define PLATFORM_SHA1_DIGEST_LENGTH SHA_DIGEST_LENGTH

#endif
