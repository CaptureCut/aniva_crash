#include <iostream>
#include <fstream>
#include <filesystem>

namespace diagnostics {

static bool exists(const std::string& path) {
    return std::filesystem::exists(path);
}

static void ensure_dir(const std::string& path) {
    if (!exists(path)) {
        std::filesystem::create_directories(path);
        std::cout << "[FIX] Created missing directory: " << path << "\n";
    }
}

static void ensure_file(const std::string& path, const std::string& default_content = "") {
    if (!exists(path)) {
        std::ofstream f(path);
        f << default_content;
        std::cout << "[FIX] Created missing file: " << path << "\n";
    }
}

static void check_dependency(const std::string& path, const std::string& warn_msg) {
    if (!exists(path)) {
        std::cout << "[WARN] " << warn_msg << "\n";
    }
}

void run() {
    std::cout << "[DIAG] Running startup diagnostics...\n";

    // --- Directories (корень проекта) ---
    ensure_dir("build");
    ensure_dir("patterns");
    ensure_dir("templates");
    ensure_dir("logs");

    // --- Files ---
    ensure_file("patterns/patterns.json", "[]");
    ensure_file("templates/templates.json", "[]");

    // --- Dependencies ---
    check_dependency("bin/d8", "d8 not found — JS executor will be disabled");
    check_dependency("/usr/local/cuda/bin/nvcc", "nvcc not found — GPU bias disabled");
    check_dependency("sandbox/sandbox", "sandbox missing — fallback executor enabled");

    std::cout << "[DIAG] Startup diagnostics complete.\n";
}

}
