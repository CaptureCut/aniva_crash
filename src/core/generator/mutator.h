#pragma once

#include <string>
#include <vector>
#include <random>

#include "memory/arena.h"

class Mutator {
public:
    // дефолтный конструктор — нужен для совместимости
    Mutator();

    // главный фикс — конструктор с Arena
    explicit Mutator(Arena& arena);

    // основная точка входа
    std::string mutate(const std::string& code);

private:
    Arena* arena_ = nullptr;      // ссылка на арену
    std::mt19937 rng_;            // RNG для мутаций

    // вспомогательные мутации
    std::string insert_random_stmt();
    std::string flip_operator(const std::string& code);
    std::string random_identifier();
};
