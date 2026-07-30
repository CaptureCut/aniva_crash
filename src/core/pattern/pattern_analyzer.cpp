#include "core/pattern/pattern_analyzer.h"

std::vector<PatternHit> PatternAnalyzer::analyze(const std::string& code) {
    std::vector<PatternHit> hits;

    auto add = [&](PatternKind kind, float weight, size_t pos) {
        hits.push_back(PatternHit{kind, weight, pos});
    };

    if (auto pos = code.find("throw"); pos != std::string::npos)
        add(PatternKind::Throw, 0.4f, pos);

    if (auto pos = code.find("for("); pos != std::string::npos)
        add(PatternKind::ForLoop, 0.2f, pos);

    if (auto pos = code.find(".map("); pos != std::string::npos)
        add(PatternKind::MapCall, 0.3f, pos);

    if (auto pos = code.find("WebAssembly"); pos != std::string::npos)
        add(PatternKind::Wasm, 0.8f, pos);

    if (auto pos = code.find("try"); pos != std::string::npos)
        add(PatternKind::TryCatch, 0.5f, pos);

    if (auto pos = code.find("gc("); pos != std::string::npos)
        add(PatternKind::GC, 0.9f, pos);

    if (hits.empty())
        add(PatternKind::Other, 0.1f, 0);

    return hits;
}

bool PatternAnalyzer::is_interesting(const std::vector<PatternHit>& hits) const {
    float sum = 0.0f;
    for (auto& h : hits) sum += h.weight;
    return sum >= 0.5f;
}
