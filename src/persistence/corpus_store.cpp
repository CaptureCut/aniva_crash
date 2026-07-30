#include "persistence/corpus_store.h"

#include <filesystem>
#include <fstream>
#include <random>

namespace fs = std::filesystem;

CorpusStore::CorpusStore(const std::string& dir)
    : dir_(dir)
{
    // создаём директорию, если её нет
    fs::create_directories(dir_);

    // загружаем существующие файлы в память
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

    // добавляем в память
    corpus_.push_back(script);

    // сохраняем на диск — имя файла по размеру корпуса
    std::string filename = dir_ + "/script_" + std::to_string(corpus_.size()) + ".js";

    std::ofstream out(filename);
    if (!out)
        return;

    out << script;
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
