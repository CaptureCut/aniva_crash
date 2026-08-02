#pragma once
#include <string>
#include <vector>

enum class PatternKind {
    // базовые
    Proxy,
    Atomics,
    Wasm,
    RegExp,
    GcPressure,
    TypedArrayOob,
    JitDeopt,
    Other,

    // новые опасные паттерны
    ProxyRec,        // рекурсивные ловушки Proxy
    WasmOob,         // wasm out-of-bounds
    RegExpCat,       // catastrophic backtracking
    TypedArray,      // обычные typedarray
    DataView,        // обычные dataview
};

struct PatternHit {
    PatternKind kind;
    float weight;       // сила паттерна
    size_t position;    // позиция в коде
};

class PatternAnalyzer {
public:
    PatternAnalyzer() = default;

    std::vector<PatternHit> analyze(const std::string& code);

    bool is_interesting(const std::vector<PatternHit>& hits) const;

    void reset();

private:
    // старые детекторы можно удалить — мы используем новую систему find_all()
    void detect_proxy(const std::string& code, std::vector<PatternHit>& out);
    void detect_atomics(const std::string& code, std::vector<PatternHit>& out);
    void detect_wasm(const std::string& code, std::vector<PatternHit>& out);
    void detect_regexp(const std::string& code, std::vector<PatternHit>& out);
    void detect_gc(const std::string& code, std::vector<PatternHit>& out);
    void detect_typedarray_oob(const std::string& code, std::vector<PatternHit>& out);
    void detect_jit_deopt(const std::string& code, std::vector<PatternHit>& out);
};
