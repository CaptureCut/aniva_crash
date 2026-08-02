#pragma once

#include <string>
#include <cstdint>

enum class TriageKind : uint8_t {
    Crash,          // severity 5
    SandboxError,   // severity 4
    Timeout,        // severity 3
    Interesting,    // severity 2
    JsException,    // severity 1
    Ok              // severity 0
};

struct TriageResult {
    TriageKind kind;
    std::string message;
    int severity;          // 0–5
    uint64_t crash_key;    // deduplication key (0 if none)

    // -----------------------------
    // Factory helpers
    // -----------------------------
    static TriageResult Crash(uint64_t key) {
        return { TriageKind::Crash,
                 "Native crash detected",
                 5,
                 key };
    }

    static TriageResult SandboxError(uint64_t key) {
        return { TriageKind::SandboxError,
                 "Sandbox failure detected",
                 4,
                 key };
    }

    static TriageResult Timeout(uint64_t key) {
        return { TriageKind::Timeout,
                 "Timeout detected",
                 3,
                 key };
    }

    static TriageResult Interesting(uint64_t key) {
        return { TriageKind::Interesting,
                 "Interesting case",
                 2,
                 key };
    }

    static TriageResult JsException() {
        return { TriageKind::JsException,
                 "JS exception detected",
                 1,
                 0 };
    }

    static TriageResult Ok() {
        return { TriageKind::Ok,
                 "No crash",
                 0,
                 0 };
    }
};
