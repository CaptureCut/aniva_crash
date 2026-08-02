#pragma once
#include <string>
#include <functional>

class Delta {
public:
    Delta() = default;

    // Строковый структурный delta‑reducer:
    // пытается удалять куски кода, сохраняя крэш
    std::string reduce(const std::string& code,
                       std::function<bool(const std::string&)> test_crash);
};
