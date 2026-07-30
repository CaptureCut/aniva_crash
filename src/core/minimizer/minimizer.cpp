#include "core/minimizer/minimizer.h"
#include <fstream>

Minimizer::Minimizer(EventBus& bus,
                     Executor& executor,
                     CrashStore& store)
    : bus_(bus)
    , executor_(executor)
    , store_(store)
{}

bool Minimizer::still_crashes(const std::string& js) {
    Script s;
    s.code = js;

    ExecResult r = executor_.execute(s);

    // любой ненормальный выход = краш
    if (r.status == ExecStatus::CRASH) return true;
    if (r.status == ExecStatus::TIMEOUT) return true;
    if (r.status == ExecStatus::SANDBOX_FAILURE) return true;
    if (r.status == ExecStatus::JS_EXCEPTION) return true;

    // stderr тоже признак краша
    if (!r.stderr_log.empty()) return true;

    return false;
}

std::string Minimizer::minimize(const std::string& code) {
    bus_.publish("minimizer:start", "minimization started");

    // разбиваем на строки
    std::vector<std::string> lines;
    {
        std::string cur;
        for (char c : code) {
            if (c == '\n') {
                lines.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        lines.push_back(cur);
    }

    std::vector<std::string> current = lines;

    // линейная минимизация
    for (size_t i = 0; i < current.size(); ) {
        std::vector<std::string> test = current;
        test.erase(test.begin() + i);

        std::string joined;
        for (auto& l : test) {
            joined += l + "\n";
        }

        if (still_crashes(joined)) {
            bus_.publish("minimizer:step", "line removed: " + std::to_string(i));
            current = test;
        } else {
            i++;
        }
    }

    // собираем результат
    std::string minimized;
    for (auto& l : current) {
        minimized += l + "\n";
    }

    bus_.publish("minimizer:end", "minimization finished");

    return minimized;
}
