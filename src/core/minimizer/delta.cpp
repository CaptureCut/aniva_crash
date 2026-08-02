#include "delta.h"

std::string Delta::reduce(const std::string& code,
                          std::function<bool(const std::string&)> test_crash)
{
    std::string current = code;

    size_t step = 16;
    while (step > 0) {
        bool changed = false;

        for (size_t i = 0; i + step < current.size(); ++i) {
            std::string candidate =
                current.substr(0, i) + current.substr(i + step);

            if (test_crash(candidate)) {
                current = candidate;
                changed = true;
                break;
            }
        }

        if (!changed)
            step /= 2;
    }

    return current;
}
