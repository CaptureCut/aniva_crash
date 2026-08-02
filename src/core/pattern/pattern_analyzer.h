#pragma once

#include <string>
#include <vector>
#include <cstdint>

enum class PatternKind {
    // базовые
    Throw,
    ForLoop,
    MapCall,
    TryCatch,

    // WASM
    Wasm,
    WasmOob,

    // TypedArray / DataView
    TypedArray,
    TypedArrayOob,

    // Proxy recursion
    Proxy,
    ProxyRec,

    // RegExp
    RegExp,
    RegExpCat,

    // JIT
    JitDeopt,

    // GC
    GC,

    // Atomics
    Atomics,

    // fallback
    Other
};

struct PatternHit {
    PatternKind kind;
    float weight;
    size_t position;
};

class PatternAnalyzer {
public:
    PatternAnalyzer() = default;

    std::vector<PatternHit> analyze(const std::string& code);
    bool is_interesting(const std::vector<PatternHit>& hits) const;

    void reset(); // нужен для HuntLoop::restart()
};
