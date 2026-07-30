#pragma once
#include <string>
#include <vector>

class Persistence {
public:
    explicit Persistence(const std::string& root_dir);

    bool save_corpus(const std::vector<std::string>& corpus);
    bool load_corpus(std::vector<std::string>& corpus);

    bool save_crashes(const std::vector<std::string>& crashes);
    bool load_crashes(std::vector<std::string>& crashes);

    bool save_bias_history(const std::vector<float>& bias);
    bool load_bias_history(std::vector<float>& bias);

private:
    std::string root_;
    bool write_file(const std::string& path, const std::string& data);
    bool read_file(const std::string& path, std::string& out);
};
