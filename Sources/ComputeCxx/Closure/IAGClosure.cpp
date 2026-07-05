#include "ComputeCxx/IAGClosure.h"

#include <swift/Runtime/HeapObject.h>

#include "ClosureFunction.h"

namespace IAG {

IAG_CLOSURE_CONTEXT_REFCOUNT_ATTR void *retain_swift_context(const void *context) noexcept {
    void *mutable_context = const_cast<void *>(context);
    return ::swift::swift_retain(reinterpret_cast<::swift::HeapObject *>(mutable_context));
}

IAG_CLOSURE_CONTEXT_REFCOUNT_ATTR void release_swift_context(const void *context) noexcept {
    void *mutable_context = const_cast<void *>(context);
    ::swift::swift_release(reinterpret_cast<::swift::HeapObject *>(mutable_context));
}

} // namespace IAG

IAGClosureStorage IAGRetainClosure(const void *thunk, const void *_Nullable context) {
    const void *retained_context = context ? IAG::retain_swift_context(context) : nullptr;
    return IAGClosureStorage((void *)thunk, retained_context);
}

void IAGReleaseClosure(IAGClosureStorage closure) {
    if (closure.context) {
        IAG::release_swift_context(closure.context);
    }
}
