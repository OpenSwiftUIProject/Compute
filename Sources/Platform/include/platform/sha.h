#pragma once

#include <platform/base.h>

#if defined(__wasi__)

typedef struct {
    uint32_t state[5];
    uint64_t bit_count;
    unsigned char buffer[64];
} PLATFORM_SHA1_CTX;

PLATFORM_EXTERN_C_BEGIN

int _platform_sha1_init(PLATFORM_SHA1_CTX *context);
int _platform_sha1_update(PLATFORM_SHA1_CTX *context, const void *data, size_t length);
int _platform_sha1_final(unsigned char digest[20], PLATFORM_SHA1_CTX *context);

PLATFORM_EXTERN_C_END

#define PLATFORM_SHA1_Init _platform_sha1_init
#define PLATFORM_SHA1_Update _platform_sha1_update
#define PLATFORM_SHA1_Final _platform_sha1_final
#define PLATFORM_SHA1_DIGEST_LENGTH 20

#elif __APPLE__

#include <CommonCrypto/CommonDigest.h>

typedef CC_SHA1_CTX PLATFORM_SHA1_CTX;

#define PLATFORM_SHA1_Init CC_SHA1_Init
#define PLATFORM_SHA1_Update CC_SHA1_Update
#define PLATFORM_SHA1_Final CC_SHA1_Final
#define PLATFORM_SHA1_DIGEST_LENGTH CC_SHA1_DIGEST_LENGTH

#else

#include <openssl/sha.h>

typedef SHA_CTX PLATFORM_SHA1_CTX;

#define PLATFORM_SHA1_Init SHA1_Init
#define PLATFORM_SHA1_Update SHA1_Update
#define PLATFORM_SHA1_Final SHA1_Final
#define PLATFORM_SHA1_DIGEST_LENGTH SHA_DIGEST_LENGTH

#endif
