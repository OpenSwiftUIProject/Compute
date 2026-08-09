#include "platform/sha.h"

#if defined(__wasi__)

#include <string.h>

static uint32_t rotate_left(uint32_t value, unsigned count) {
    return (value << count) | (value >> (32 - count));
}

static uint32_t load_big_endian(const unsigned char *bytes) {
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) |
           (uint32_t)bytes[3];
}

static void process_block(PLATFORM_SHA1_CTX *context, const unsigned char block[64]) {
    uint32_t words[80];
    for (unsigned i = 0; i < 16; ++i) {
        words[i] = load_big_endian(block + i * 4);
    }
    for (unsigned i = 16; i < 80; ++i) {
        words[i] = rotate_left(words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16], 1);
    }

    uint32_t a = context->state[0];
    uint32_t b = context->state[1];
    uint32_t c = context->state[2];
    uint32_t d = context->state[3];
    uint32_t e = context->state[4];

    for (unsigned i = 0; i < 80; ++i) {
        uint32_t function;
        uint32_t constant;
        if (i < 20) {
            function = (b & c) | ((~b) & d);
            constant = 0x5a827999;
        } else if (i < 40) {
            function = b ^ c ^ d;
            constant = 0x6ed9eba1;
        } else if (i < 60) {
            function = (b & c) | (b & d) | (c & d);
            constant = 0x8f1bbcdc;
        } else {
            function = b ^ c ^ d;
            constant = 0xca62c1d6;
        }

        uint32_t temporary = rotate_left(a, 5) + function + e + constant + words[i];
        e = d;
        d = c;
        c = rotate_left(b, 30);
        b = a;
        a = temporary;
    }

    context->state[0] += a;
    context->state[1] += b;
    context->state[2] += c;
    context->state[3] += d;
    context->state[4] += e;
}

int _platform_sha1_init(PLATFORM_SHA1_CTX *context) {
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
    context->state[4] = 0xc3d2e1f0;
    context->bit_count = 0;
    memset(context->buffer, 0, sizeof(context->buffer));
    return 1;
}

int _platform_sha1_update(PLATFORM_SHA1_CTX *context, const void *data, size_t length) {
    const unsigned char *bytes = data;
    size_t buffered = (size_t)((context->bit_count >> 3) & 63);
    context->bit_count += (uint64_t)length << 3;

    if (buffered != 0) {
        size_t available = 64 - buffered;
        size_t copied = length < available ? length : available;
        memcpy(context->buffer + buffered, bytes, copied);
        buffered += copied;
        bytes += copied;
        length -= copied;
        if (buffered == 64) {
            process_block(context, context->buffer);
        }
    }

    while (length >= 64) {
        process_block(context, bytes);
        bytes += 64;
        length -= 64;
    }

    if (length != 0) {
        memcpy(context->buffer, bytes, length);
    }
    return 1;
}

int _platform_sha1_final(unsigned char digest[20], PLATFORM_SHA1_CTX *context) {
    uint64_t original_bit_count = context->bit_count;
    unsigned char padding[64] = {0x80};
    size_t buffered = (size_t)((original_bit_count >> 3) & 63);
    size_t padding_length = buffered < 56 ? 56 - buffered : 120 - buffered;
    _platform_sha1_update(context, padding, padding_length);

    unsigned char encoded_length[8];
    for (unsigned i = 0; i < 8; ++i) {
        encoded_length[7 - i] = (unsigned char)(original_bit_count >> (i * 8));
    }
    _platform_sha1_update(context, encoded_length, sizeof(encoded_length));

    for (unsigned i = 0; i < 5; ++i) {
        digest[i * 4] = (unsigned char)(context->state[i] >> 24);
        digest[i * 4 + 1] = (unsigned char)(context->state[i] >> 16);
        digest[i * 4 + 2] = (unsigned char)(context->state[i] >> 8);
        digest[i * 4 + 3] = (unsigned char)context->state[i];
    }
    memset(context, 0, sizeof(*context));
    return 1;
}

#endif
