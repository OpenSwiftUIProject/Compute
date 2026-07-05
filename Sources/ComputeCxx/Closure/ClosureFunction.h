#pragma once

#include <swift/Runtime/HeapObject.h>

#include "ComputeCxx/IAGBase.h"

#if __has_attribute(noinline)
#define IAG_CLOSURE_CONTEXT_NOINLINE __attribute__((noinline))
#else
#define IAG_CLOSURE_CONTEXT_NOINLINE
#endif

#if __has_attribute(optnone)
#define IAG_CLOSURE_CONTEXT_OPTNONE __attribute__((optnone))
#else
#define IAG_CLOSURE_CONTEXT_OPTNONE
#endif

#define IAG_CLOSURE_CONTEXT_REFCOUNT_ATTR IAG_CLOSURE_CONTEXT_NOINLINE IAG_CLOSURE_CONTEXT_OPTNONE

namespace IAG {

IAG_CLOSURE_CONTEXT_REFCOUNT_ATTR void *_Nullable retain_swift_context(const void *_Nullable context) noexcept;
IAG_CLOSURE_CONTEXT_REFCOUNT_ATTR void release_swift_context(const void *_Nullable context) noexcept;

/// C++ function type that is equivalent to the lowered Swift closure type.
template <typename Result, typename... Args> class ClosureFunction {
  public:
    using Context = const void *_Nullable;
    using Function = IAG_SWIFT_CC(swift) Result (*_Nullable)(Args..., Context IAG_SWIFT_CONTEXT);

  private:
    Function _function;
    Context _context;

  public:
    inline ClosureFunction(std::nullptr_t): _function(nullptr), _context(nullptr) {}
    inline ClosureFunction(Function function, Context context) noexcept : _function(function), _context(context) {
        if (_context) {
            _context = retain_swift_context(_context);
        }
    }

    inline ~ClosureFunction() {
        if (_context) {
            release_swift_context(_context);
        }
    }

    // Copyable

    ClosureFunction(const ClosureFunction &other) noexcept : _function(other._function), _context(other._context) {
        if (_context) {
            _context = retain_swift_context(_context);
        }
    };

    ClosureFunction &operator=(const ClosureFunction &other) noexcept {
        if (this != &other) {
            Context new_context = other._context;
            if (new_context) {
                new_context = retain_swift_context(new_context);
            }
            Context old_context = _context;
            _function = other._function;
            _context = new_context;
            if (old_context) {
                release_swift_context(old_context);
            }
        }
        return *this;
    };

    // Move

    ClosureFunction(ClosureFunction &&other) noexcept
        : _function(std::exchange(other._function, nullptr)), _context(std::exchange(other._context, nullptr)) {};

    ClosureFunction &operator=(ClosureFunction &&other) noexcept {
        if (this != &other) {
            Context old_context = _context;
            _function = other._function;
            _context = other._context;
            other._function = nullptr;
            other._context = nullptr;
            if (old_context) {
                release_swift_context(old_context);
            }
        }
        return *this;
    }

    explicit operator bool() { return _function != nullptr; }

    const Result operator()(Args... args) const noexcept {
        return _function(std::forward<Args>(args)..., _context);
    }
};

template <typename Result>
    requires std::is_pointer_v<Result>
using ClosureFunctionVP = ClosureFunction<Result>;

template <typename Result>
    requires std::same_as<Result, void>
using ClosureFunctionVV = ClosureFunction<Result>;

template <typename Result, typename Arg>
    requires std::same_as<Result, void> && std::unsigned_integral<Arg>
using ClosureFunctionAV = ClosureFunction<Result, Arg>;

template <typename Result, typename Arg>
    requires std::same_as<Result, bool> && std::unsigned_integral<Arg>
using ClosureFunctionAB = ClosureFunction<Result, Arg>;

template <typename Result, typename Arg>
    requires std::same_as<Result, void>
using ClosureFunctionPV = ClosureFunction<Result, Arg>;

template <typename Result, typename Arg>
    requires std::unsigned_integral<Result>
using ClosureFunctionCI = ClosureFunction<Result, Arg>;

} // namespace IAG
