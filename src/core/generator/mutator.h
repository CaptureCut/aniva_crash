#pragma once

#include <string>
#include <vector>
#include <random>

#include "memory/arena.h"

class Mutator {
public:
    // дефолтный конструктор — нужен для совместимости
    Mutator();

    // конструктор с Arena
    explicit Mutator(Arena& arena);

    // основная точка входа
    std::string mutate(const std::string& code);

private:
    Arena* arena_ = nullptr;      // ссылка на арену
    std::mt19937 rng_;            // RNG для мутаций

    // безопасные мутации
    std::string insert_safe_stmt(const std::string& code);
    std::string flip_safe_operator(const std::string& code);
    std::string wrap_in_try(const std::string& code);
    std::string append_function_call(const std::string& code);
    std::string append_wasm_trigger(const std::string& code);
};
