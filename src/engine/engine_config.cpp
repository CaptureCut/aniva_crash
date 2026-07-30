#include "engine_config.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static std::string read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("cannot open config file: " + path);
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

EngineConfig load_engine_config(const std::string& path) {
    std::string raw = read_file(path);
    json j = json::parse(raw);

    EngineConfig cfg;

    // обязательные поля
    cfg.engine_path = j.value("engine_path", "");
    cfg.corpus_dir  = j.value("corpus_dir", "");
    cfg.crash_dir   = j.value("crash_dir", "");

    if (cfg.engine_path.empty())
        throw std::runtime_error("engine_path missing in config");

    if (cfg.corpus_dir.empty())
        throw std::runtime_error("corpus_dir missing in config");

    if (cfg.crash_dir.empty())
        throw std::runtime_error("crash_dir missing in config");

    // необязательные поля
    cfg.timeout_ms   = j.value("timeout_ms", 200);
    cfg.heartbeat_ms = j.value("heartbeat_ms", 1000);
    cfg.gpu_config   = j.value("gpu_config", "");

    return cfg;
}
