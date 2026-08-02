#pragma once

#include <string>
#include <vector>

#include "bus/event_bus.h"
#include "core/triage/triage.h"
#include "core/executor/executor.h"
#include "core/pattern/pattern_analyzer.h"

class CorpusStore {
public:
    explicit CorpusStore(const std::string& dir, EventBus& bus);

    // старый метод — оставляем для совместимости
    void save(const std::string& script);

    // новый метод — авто‑сортировка кейсов
    void save_case(const std::string& js_code,
                   ExecStatus exec_status,
                   const TriageResult& triage,
                   const std::vector<PatternHit>& patterns);

    std::string get_random() const;
    size_t size() const;

private:
    std::string dir_;
    std::vector<std::string> corpus_;
    EventBus& bus_;
};
