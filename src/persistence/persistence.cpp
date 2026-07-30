#include "persistence.h"
#include <filesystem>
#include <fstream>

Persistence::Persistence(const std::string& root_dir)
    : root_(root_dir)
{
    std::filesystem::create_directories(root_);
}

bool Persistence::write_file(const std::string& path, const std::string& data) {
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) return false;
    f.write(data.data(), data.size());
    return true;
}

bool Persistence::read_file(const std::string& path, std::string& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return false;
    out.assign(std::istreambuf_iterator<char>(f), {});
    return true;
}

bool Persistence::save_corpus(const std::vector<std::string>& corpus) {
    std::string data;
    for (auto& s : corpus) {
        data += s;
        data += "\n---\n";
    }
    return write_file(root_ + "/corpus.txt", data);
}

bool Persistence::load_corpus(std::vector<std::string>& corpus) {
    std::string data;
    if (!read_file(root_ + "/corpus.txt", data)) return false;

    corpus.clear();
    size_t pos = 0;
    while (true) {
        size_t sep = data.find("\n---\n", pos);
        if (sep == std::string::npos) break;
        corpus.push_back(data.substr(pos, sep - pos));
        pos = sep + 5;
    }
    return true;
}

bool Persistence::save_crashes(const std::vector<std::string>& crashes) {
    std::string data;
    for (auto& s : crashes) {
        data += s;
        data += "\n===\n";
    }
    return write_file(root_ + "/crashes.txt", data);
}

bool Persistence::load_crashes(std::vector<std::string>& crashes) {
    std::string data;
    if (!read_file(root_ + "/crashes.txt", data)) return false;

    crashes.clear();
    size_t pos = 0;
    while (true) {
        size_t sep = data.find("\n===\n", pos);
        if (sep == std::string::npos) break;
        crashes.push_back(data.substr(pos, sep - pos));
        pos = sep + 5;
    }
    return true;
}

bool Persistence::save_bias_history(const std::vector<float>& bias) {
    std::string data;
    for (float b : bias) {
        data += std::to_string(b);
        data += "\n";
    }
    return write_file(root_ + "/bias.txt", data);
}

bool Persistence::load_bias_history(std::vector<float>& bias) {
    std::string data;
    if (!read_file(root_ + "/bias.txt", data)) return false;

    bias.clear();
    size_t pos = 0;
    while (true) {
        size_t nl = data.find('\n', pos);
        if (nl == std::string::npos) break;
        bias.push_back(std::stof(data.substr(pos, nl - pos)));
        pos = nl + 1;
    }
    return true;
}
