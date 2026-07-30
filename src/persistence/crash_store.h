#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "persistence/crash_entry.h"

class CrashStore {
public:
    explicit CrashStore(const std::string& base_dir);

    // сохраняет краш и возвращает путь к JS-файлу
    std::string save(const CrashEntry& entry);

    const std::vector<CrashEntry>& all() const;
    size_t size() const;

private:
    std::filesystem::path base_dir_;
    std::vector<CrashEntry> entries_;

    void ensure_dir();
    std::string make_filename(const CrashEntry& entry);
};
