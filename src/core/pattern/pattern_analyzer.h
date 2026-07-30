#pragma once

#include <string>
#include <vector>

// типы паттернов, которые реально используются в движке
enum class PatternKind {
    Throw,
    ForLoop,
    MapCall,
    Wasm,
    TryCatch,
    GC,
    Other
};

// единичное совпадение паттерна
struct PatternHit {
    PatternKind kind;   // тип паттерна
    float weight;       // сила паттерна (для bias)
    size_t position;    // позиция в JS-коде
};

class PatternAnalyzer {
public:
    PatternAnalyzer() = default;

    // анализ JS-кода → список паттернов
    std::vector<PatternHit> analyze(const std::string& code);

    // простой критерий "интересности"
    bool is_interesting(const std::vector<PatternHit>& hits) const;
};
