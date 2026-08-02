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

void run() {
    std::cout << "[DIAG] Running startup diagnostics...\n";

    ensure_dir("build");
    ensure_dir("patterns");
    ensure_dir("templates");
    ensure_dir("logs");

    ensure_file("patterns/patterns.json", "[]");
    ensure_file("templates/templates.json", "[]");

    std::cout << "[OK] diagnostics: basic project structure verified\n";
    std::cout << "[DIAG] Startup diagnostics complete.\n";
}

}
