#pragma once
#include <string>
#include <vector>

enum class PatternKind {
    Proxy,
    Atomics,
    Wasm,
    RegExp,
    GcPressure,
    TypedArrayOob,
    JitDeopt,
    Unknown
};

struct PatternHit {
    PatternKind kind;
    float weight;       // сила паттерна
    size_t position;    // позиция в коде
};

class PatternAnalyzer {
public:
    PatternAnalyzer() = default;

    // анализирует JS-код и возвращает найденные паттерны
    std::vector<PatternHit> analyze(const std::string& code);

private:
    void detect_proxy(const std::string& code, std::vector<PatternHit>& out);
    void detect_atomics(const std::string& code, std::vector<PatternHit>& out);
    void detect_wasm(const std::string& code, std::vector<PatternHit>& out);
    void detect_regexp(const std::string& code, std::vector<PatternHit>& out);
    void detect_gc(const std::string& code, std::vector<PatternHit>& out);
    void detect_typedarray_oob(const std::string& code, std::vector<PatternHit>& out);
    void detect_jit_deopt(const std::string& code, std::vector<PatternHit>& out);
};
