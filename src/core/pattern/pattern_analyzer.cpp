#include "core/pattern/pattern_analyzer.h"

std::vector<PatternHit> PatternAnalyzer::analyze(const std::string& code) {
    std::vector<PatternHit> hits;

    auto add = [&](PatternKind kind, float weight, size_t pos) {
        hits.push_back(PatternHit{kind, weight, pos});
    };

    auto find_all = [&](const std::string& needle, PatternKind kind, float weight) {
        size_t pos = code.find(needle);
        while (pos != std::string::npos) {
            add(kind, weight, pos);
            pos = code.find(needle, pos + needle.size());
        }
    };

    // -----------------------------------------
    // базовые паттерны
    // -----------------------------------------
    find_all("throw",        PatternKind::Throw,         0.4f);
    find_all("for(",         PatternKind::ForLoop,       0.2f);
    find_all(".map(",        PatternKind::MapCall,       0.3f);
    find_all("try",          PatternKind::TryCatch,      0.5f);

    // -----------------------------------------
    // WASM
    // -----------------------------------------
    find_all("WebAssembly",          PatternKind::Wasm,          0.8f);
    find_all("i32.load",             PatternKind::WasmOob,       1.0f);
    find_all("i32.store",            PatternKind::WasmOob,       1.0f);
    find_all("memory",               PatternKind::WasmOob,       0.9f);
    find_all("table",                PatternKind::WasmOob,       0.9f);

    // -----------------------------------------
    // TypedArray / DataView OOB
    // -----------------------------------------
    find_all("Uint8Array",           PatternKind::TypedArray,    0.3f);
    find_all("DataView",             PatternKind::TypedArray,    0.3f);
    find_all("99999999",             PatternKind::TypedArrayOob, 1.0f);
    find_all("NaN",                  PatternKind::TypedArrayOob, 0.8f);
    find_all("Infinity",             PatternKind::TypedArrayOob, 0.8f);
    find_all("Symbol(",              PatternKind::TypedArrayOob, 0.7f);
    find_all("setUint32",            PatternKind::TypedArrayOob, 0.9f);
    find_all("getFloat64",           PatternKind::TypedArrayOob, 0.9f);

    // -----------------------------------------
    // Proxy recursion
    // -----------------------------------------
    find_all("new Proxy",            PatternKind::Proxy,         0.7f);
    find_all("recv[prop]",           PatternKind::ProxyRec,      1.0f);
    find_all("new Proxy(o,h)",       PatternKind::ProxyRec,      1.0f);

    // -----------------------------------------
    // RegExp catastrophic backtracking
    // -----------------------------------------
    find_all("RegExp(",              PatternKind::RegExp,        0.4f);
    find_all("(a+)+$",               PatternKind::RegExpCat,     1.0f);
    find_all("repeat(5000)",         PatternKind::RegExpCat,     1.0f);

    // -----------------------------------------
    // JIT deopt storms
    // -----------------------------------------
    find_all("typeof x",             PatternKind::JitDeopt,      0.8f);
    find_all("obj['k' + i]",         PatternKind::JitDeopt,      0.9f);
    find_all("hot(",                 PatternKind::JitDeopt,      0.9f);

    // -----------------------------------------
    // GC pressure
    // -----------------------------------------
    find_all("new Array(1000)",      PatternKind::GC,            0.9f);
    find_all("gc()",                 PatternKind::GC,            1.0f);

    // -----------------------------------------
    // Atomics
    // -----------------------------------------
    find_all("Atomics.",             PatternKind::Atomics,       0.6f);
    find_all("SharedArrayBuffer",    PatternKind::Atomics,       0.7f);

    // -----------------------------------------
    // fallback
    // -----------------------------------------
    if (hits.empty())
        add(PatternKind::Other, 0.1f, 0);

    return hits;
}

bool PatternAnalyzer::is_interesting(const std::vector<PatternHit>& hits) const {
    float score = 0.0f;
    for (auto& h : hits)
        score += h.weight;

    // более агрессивный критерий
    return score >= 1.0f || hits.size() >= 3;
}

void PatternAnalyzer::reset() {
    // пусто — но нужно для HuntLoop::restart()
}
