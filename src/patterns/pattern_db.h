#pragma once
#include <string>
#include <vector>
#include <unordered_map>

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

struct PatternInfo {
    PatternKind kind;
    std::string name;
    float base_weight;
    std::vector<std::string> tokens;   // ключевые слова
};

class PatternDB {
public:
    PatternDB();

    const PatternInfo* get(PatternKind kind) const;
    const PatternInfo* get(const std::string& name) const;

    const std::vector<PatternInfo>& all() const { return patterns_; }

private:
    std::vector<PatternInfo> patterns_;
    std::unordered_map<std::string, size_t> name_index_;
    std::unordered_map<PatternKind, size_t> kind_index_;

    void add(const PatternInfo& info);
};
