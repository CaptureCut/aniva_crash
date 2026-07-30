#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "core/pattern/pattern_analyzer.h"
#include "core/executor/executor_result.h"

struct AnomalyEntry {
    std::string code;                 // JS-код
    std::vector<PatternHit> patterns; // паттерны
    ExecResult exec;                  // результат выполнения
    std::time_t timestamp;            // время обнаружения
};

class AnomalyStore {
public:
    explicit AnomalyStore(const std::string& dir);

    void save(const AnomalyEntry& entry);

    const std::vector<AnomalyEntry>& all() const;
    size_t size() const;

private:
    std::filesystem::path dir_;
    std::vector<AnomalyEntry> entries_;
};
