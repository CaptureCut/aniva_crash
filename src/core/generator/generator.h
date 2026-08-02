#pragma once

#include <string>
#include <vector>

#include "bus/event_bus.h"
#include "core/generator/script.h"
#include "core/generator/mutator.h"
#include "core/pattern/pattern_analyzer.h"
#include "gpu/gpu_bias.h"
#include "persistence/corpus_store.h"
#include "memory/arena.h"

class Generator {
public:
    Generator(EventBus& bus,
              GpuBias* gpu_bias,
              CorpusStore* corpus);

    // основной метод генерации JS-кода
    Script generate();

    // обновление bias по паттернам (feedback loop)
    void update_bias(const std::vector<PatternHit>& patterns);

    // нужен для HuntLoop::restart()
    void reset();

private:
    // базовый seed или случайный из корпуса
    std::string build_seed();

    // GPU bias → вставка JS‑конструкций
    void apply_gpu_bias(std::string& code,
                        const GpuBiasResult& bias);

    // Pattern bias → вставка JS‑конструкций
    void apply_pattern_bias(std::string& code,
                            const std::vector<PatternHit>& patterns);

    EventBus& bus_;
    GpuBias* gpu_bias_;
    CorpusStore* corpus_;

    Arena arena_;
    Mutator mutator_;

    // анализатор паттернов
    PatternAnalyzer analyzer_;
};
