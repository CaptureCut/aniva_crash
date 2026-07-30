#include "pattern_db.h"

PatternDB::PatternDB() {
    add({PatternKind::Proxy, "proxy", 0.4f, {"Proxy", "handler", "get"}});
    add({PatternKind::Atomics, "atomics", 0.5f, {"Atomics", "SharedArrayBuffer"}});
    add({PatternKind::Wasm, "wasm", 0.6f, {"WebAssembly", "instantiate"}});
    add({PatternKind::RegExp, "regexp", 0.3f, {"RegExp", "exec", "match"}});
    add({PatternKind::GcPressure, "gc", 0.7f, {"gc()", "WeakRef", "FinalizationRegistry"}});
    add({PatternKind::TypedArrayOob, "typedarray_oob", 0.8f, {"Uint8Array", "Uint32Array", "length +"}});
    add({PatternKind::JitDeopt, "jit_deopt", 0.9f, {"%OptimizeFunctionOnNextCall", "%DeoptimizeFunction"}});
}

void PatternDB::add(const PatternInfo& info) {
    size_t idx = patterns_.size();
    patterns_.push_back(info);

    name_index_[info.name] = idx;
    kind_index_[info.kind] = idx;
}

const PatternInfo* PatternDB::get(PatternKind kind) const {
    auto it = kind_index_.find(kind);
    if (it == kind_index_.end()) return nullptr;
    return &patterns_[it->second];
}

const PatternInfo* PatternDB::get(const std::string& name) const {
    auto it = name_index_.find(name);
    if (it == name_index_.end()) return nullptr;
    return &patterns_[it->second];
}
