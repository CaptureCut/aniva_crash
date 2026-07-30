#include "crash_store.h"

#include <fstream>
#include <sstream>
#include <iomanip>

CrashStore::CrashStore(const std::string& base_dir)
    : base_dir_(std::filesystem::path(base_dir))
{
    ensure_dir();
}

void CrashStore::ensure_dir() {
    if (!std::filesystem::exists(base_dir_)) {
        std::filesystem::create_directories(base_dir_);
    }
}

static std::string escape_json(const std::string& s) {
    std::ostringstream oss;
    for (char c : s) {
        switch (c) {
            case '\"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    oss << "\\u"
                        << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(c);
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

std::string CrashStore::make_filename(const CrashEntry& entry) {
    std::ostringstream oss;
    oss << "crash_" << entry.signature;
    return oss.str();
}

std::string CrashStore::save(const CrashEntry& entry) {
    ensure_dir();

    const std::string base = make_filename(entry);

    //
    // 1) Save original JS
    //
    std::filesystem::path js_path = base_dir_ / (base + ".js");
    {
        std::ofstream js(js_path);
        js << entry.original;
    }

    //
    // 2) Save JSON metadata
    //
    std::filesystem::path meta_path = base_dir_ / (base + ".json");
    {
        std::ofstream meta(meta_path);

        meta << "{\n";
        meta << "  \"signature\": \"" << escape_json(entry.signature) << "\",\n";
        meta << "  \"timestamp\": " << entry.timestamp << ",\n";

        meta << "  \"exec\": {\n";
        meta << "    \"exit_code\": " << entry.exec.exit_code << ",\n";
        meta << "    \"signal\": " << entry.exec.signal << ",\n";
        meta << "    \"status\": " << static_cast<int>(entry.exec.status) << ",\n";
        meta << "    \"stdout\": \"" << escape_json(entry.exec.stdout_log) << "\",\n";
        meta << "    \"stderr\": \"" << escape_json(entry.exec.stderr_log) << "\"\n";
        meta << "  },\n";

        meta << "  \"patterns\": [\n";
        for (size_t i = 0; i < entry.patterns.size(); i++) {
            const auto& p = entry.patterns[i];
            meta << "    {"
                 << "\"kind\": " << static_cast<int>(p.kind) << ", "
                 << "\"weight\": " << p.weight << ", "
                 << "\"position\": " << p.position
                 << "}";
            if (i + 1 < entry.patterns.size()) meta << ",";
            meta << "\n";
        }
        meta << "  ],\n";

        meta << "  \"js_path\": \"" << escape_json(js_path.string()) << "\",\n";
        meta << "  \"minimized\": \"" << escape_json(entry.minimized) << "\"\n";

        meta << "}\n";
    }

    entries_.push_back(entry);
    return js_path.string();
}

const std::vector<CrashEntry>& CrashStore::all() const {
    return entries_;
}

size_t CrashStore::size() const {
    return entries_.size();
}
