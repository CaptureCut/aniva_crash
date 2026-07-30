#include "pattern_analyzer.h"

std::vector<PatternHit> PatternAnalyzer::analyze(const std::string& code) {
    std::vector<PatternHit> hits;

    detect_proxy(code, hits);
    detect_atomics(code, hits);
    detect_wasm(code, hits);
    detect_regexp(code, hits);
    detect_gc(code, hits);
    detect_typedarray_oob(code, hits);
    detect_jit_deopt(code, hits);

    return hits;
}

void PatternAnalyzer::detect_proxy(const std::string& code, std::vector<PatternHit>& out) {
    auto pos = code.find("Proxy");
    if (pos != std::string::npos) {
        out.push_back({PatternKind::Proxy, 0.4f, pos});
    }
}

void PatternAnalyzer::detect_atomics(const std::string& code, std::vector<PatternHit>& out) {
    auto pos = code.find("Atomics");
    if (pos != std::string::npos) {
        out.push_back({PatternKind::Atomics, 0.5f, pos});
    }
}

void PatternAnalyzer::detect_wasm(const std::string& code, std::vector<PatternHit>& out) {
    auto pos = code.find("WebAssembly");
    if (pos != std::string::npos) {
        out.push_back({PatternKind::Wasm, 0.6f, pos});
    }
}

void PatternAnalyzer::detect_regexp(const std::string& code, std::vector<PatternHit>& out) {
    auto pos = code.find("RegExp");
    if (pos != std::string::npos) {
        out.push_back({PatternKind::RegExp, 0.3f, pos});
    }
}

void PatternAnalyzer::detect_gc(const std::string& code, std::vector<PatternHit>& out) {
    auto pos = code.find("gc()");
    if (pos != std::string::npos) {
        out.push_back({PatternKind::GcPressure, 0.7f, pos});
    }
}

void PatternAnalyzer::detect_typedarray_oob(const std::string& code, std::vector<PatternHit>& out) {
    auto pos = code.find("new Uint8Array(");
    if (pos != std::string::npos && code.find("length + 100") != std::string::npos) {
        out.push_back({PatternKind::TypedArrayOob, 0.8f, pos});
    }
}

void PatternAnalyzer::detect_jit_deopt(const std::string& code, std::vector<PatternHit>& out) {
    auto pos = code.find("%OptimizeFunctionOnNextCall");
    if (pos != std::string::npos) {
        out.push_back({PatternKind::JitDeopt, 0.9f, pos});
    }
}
