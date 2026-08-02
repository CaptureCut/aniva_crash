#include "persistence/corpus_store.h"

#include <filesystem>
#include <fstream>
#include <random>
#include <chrono>

namespace fs = std::filesystem;

static void ensure_dir(const std::string& path) {
    if (!fs::exists(path)) fs::create_directories(path);
}

static std::string timestamp() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    return std::to_string(t);
}

static std::string classify_case(const ExecStatus exec_status,
                                 const TriageResult& triage,
                                 const std::vector<PatternHit>& patterns)
{
    if (exec_status == ExecStatus::CRASH)
        return "corpus/crashes/";

    if (exec_status == ExecStatus::INTERESTING)
        return "corpus/interesting/";

    // классификация по паттернам
    for (const auto& p : patterns) {

        if (p.kind == PatternKind::Wasm)
            return "corpus/wasm/";

        if (p.kind == PatternKind::JitDeopt)
            return "corpus/jit/";
    }

    return "corpus/interesting/";
}

CorpusStore::CorpusStore(const std::string& dir, EventBus& bus)
    : dir_(dir)
    , bus_(bus)
{
    ensure_dir(dir_);

    for (const auto& entry : fs::directory_iterator(dir_)) {
        if (!entry.is_regular_file()) continue;

        std::ifstream in(entry.path());
        if (!in) continue;

        std::string code((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());

        if (!code.empty())
            corpus_.push_back(code);
    }
}

void CorpusStore::save(const std::string& script) {
    if (script.empty())
        return;

    corpus_.push_back(script);

    std::string filename = dir_ + "/script_" + std::to_string(corpus_.size()) + ".js";

    std::ofstream out(filename);
    if (!out)
        return;

    out << script;
}

void CorpusStore::save_case(const std::string& js_code,
                            const ExecStatus exec_status,
                            const TriageResult& triage,
                            const std::vector<PatternHit>& patterns)
{
    std::string subdir = classify_case(exec_status, triage, patterns);
    ensure_dir(subdir);

    std::string file = subdir + timestamp() + ".js";

    std::ofstream out(file);
    out << js_code;
    out.close();

    bus_.publish("corpus:save", "Saved case to: " + file);
}

std::string CorpusStore::get_random() const {
    if (corpus_.empty())
        return {};

    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<size_t> dist(0, corpus_.size() - 1);

    return corpus_[dist(rng)];
}

size_t CorpusStore::size() const {
    return corpus_.size();
}
