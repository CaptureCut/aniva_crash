#include "persistence/anomaly_store.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

AnomalyStore::AnomalyStore(const std::string& dir)
    : dir_(fs::path(dir))
{
    fs::create_directories(dir_);
}

void AnomalyStore::save(const AnomalyEntry& entry) {
    entries_.push_back(entry);

    // имя файла: anomaly_<N>.txt
    fs::path filename = dir_ / ("anomaly_" + std::to_string(entries_.size()) + ".txt");

    std::ofstream out(filename);
    if (!out)
        return;

    out << "timestamp=" << entry.timestamp << "\n";
    out << "exit_code=" << entry.exec.exit_code << "\n";
    out << "signal=" << entry.exec.signal << "\n";
    out << "status=" << static_cast<int>(entry.exec.status) << "\n";
    out << "stdout=" << entry.exec.stdout_log << "\n";
    out << "stderr=" << entry.exec.stderr_log << "\n";

    out << "\npatterns:\n";
    for (const auto& p : entry.patterns) {
        out << "  kind=" << static_cast<int>(p.kind)
            << " weight=" << p.weight
            << " position=" << p.position << "\n";
    }

    out << "\ncode:\n";
    out << entry.code << "\n";
}

const std::vector<AnomalyEntry>& AnomalyStore::all() const {
    return entries_;
}

size_t AnomalyStore::size() const {
    return entries_.size();
}
