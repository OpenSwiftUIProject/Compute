// Adapted from Swift's lib/Demangling/Demangler.cpp.
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors.
// Licensed under Apache License v2.0 with Runtime Library Exception.
// See https://swift.org/LICENSE.txt for license information.

#if defined(__ANDROID__)

#include <swift/Demangling/Demangle.h>

// The Android SDK does not ship libswiftDemangle. Preserve embedded null bytes
// in symbolic reference payloads when forming the runtime metadata lookup name.
llvm::StringRef swift::Demangle::makeSymbolicMangledNameStringRef(const char *base) {
    if (!base) {
        return {};
    }

    auto end = base;
    while (*end != '\0') {
        if (*end >= '\x01' && *end <= '\x17') {
            end += sizeof(uint32_t);
        } else if (*end >= '\x18' && *end <= '\x1F') {
            end += sizeof(void *);
        }
        ++end;
    }
    return llvm::StringRef(base, end - base);
}

#endif
