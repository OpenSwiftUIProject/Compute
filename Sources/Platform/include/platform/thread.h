#pragma once

#include <platform/base.h>

#if !defined(__wasi__)
#include <pthread.h>
#endif

PLATFORM_ASSUME_NONNULL_BEGIN

#if defined(__wasi__)

typedef uintptr_t platform_thread_t;
typedef struct {
    void *_Nullable value;
} platform_thread_key_t;

#define PLATFORM_THREAD_NULL ((platform_thread_t)0)

PLATFORM_INLINE
int platform_thread_key_create(platform_thread_key_t *key) {
    key->value = NULL;
    return 0;
}

PLATFORM_INLINE
void *_Nullable platform_thread_getspecific(platform_thread_key_t *key) {
    return key->value;
}

PLATFORM_INLINE
int platform_thread_setspecific(platform_thread_key_t *key, const void *_Nullable value) {
    key->value = (void *)value;
    return 0;
}

PLATFORM_INLINE
platform_thread_t platform_thread_current(void) {
    return 1;
}

#else

typedef pthread_t platform_thread_t;
typedef pthread_key_t platform_thread_key_t;

#define PLATFORM_THREAD_NULL ((platform_thread_t)0)

PLATFORM_INLINE
int platform_thread_key_create(platform_thread_key_t *key) {
    return pthread_key_create(key, NULL);
}

PLATFORM_INLINE
void *_Nullable platform_thread_getspecific(platform_thread_key_t *key) {
    return pthread_getspecific(*key);
}

PLATFORM_INLINE
int platform_thread_setspecific(platform_thread_key_t *key, const void *_Nullable value) {
    return pthread_setspecific(*key, value);
}

PLATFORM_INLINE
platform_thread_t platform_thread_current(void) {
    return pthread_self();
}

#endif

PLATFORM_ASSUME_NONNULL_END
