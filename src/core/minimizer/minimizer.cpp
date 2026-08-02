#include "core/minimizer/minimizer.h"
#include "core/minimizer/ast_min.h"
#include "core/minimizer/delta.h"
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

    if (r.status == ExecStatus::CRASH) return true;
    if (r.status == ExecStatus::TIMEOUT) return true;
    if (r.status == ExecStatus::SANDBOX_FAILURE) return true;
    if (r.status == ExecStatus::JS_EXCEPTION) return true;

    if (!r.stderr_log.empty()) return true;

    return false;
}

std::string Minimizer::minimize(const std::string& code) {
    bus_.publish("minimizer:start", "minimization started");

    auto test_crash = [&](const std::string& js) {
        return still_crashes(js);
    };

    // ------------------------------------------------------------
    // 1) Линейная минимизация (как раньше)
    // ------------------------------------------------------------
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

    for (size_t i = 0; i < current.size(); ) {
        std::vector<std::string> test = current;
        test.erase(test.begin() + i);

        std::string joined;
        for (auto& l : test)
            joined += l + "\n";

        if (still_crashes(joined)) {
            bus_.publish("minimizer:step", "line removed: " + std::to_string(i));
            current = test;
        } else {
            i++;
        }
    }

    std::string minimized;
    for (auto& l : current)
        minimized += l + "\n";

    // ------------------------------------------------------------
    // 2) Структурный delta‑reducer (delta.cpp)
    // ------------------------------------------------------------
    bus_.publish("minimizer:delta", "structural delta started");

    Delta delta;
    minimized = delta.reduce(minimized, test_crash);

    // ------------------------------------------------------------
    // 3) AST‑минимизация (ast_min.cpp)
    // ------------------------------------------------------------
    bus_.publish("minimizer:ast", "AST minimization started");

    AstMin ast;
    minimized = ast.minimize(minimized, test_crash);

    // ------------------------------------------------------------
    // 4) Финал
    // ------------------------------------------------------------
    bus_.publish("minimizer:end", "minimization finished");

    return minimized;
}
