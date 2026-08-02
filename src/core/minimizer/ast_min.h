#pragma once
#include <string>
#include <vector>

class AstMin {
public:
    AstMin() = default;

    // Пытается минимизировать JS-код, сохраняя крэш
    std::string minimize(const std::string& code,
                         std::function<bool(const std::string&)> test_crash);

private:
    // Псевдо-узел AST
    struct Node {
        size_t start;
        size_t end;
    };

    std::vector<Node> find_blocks(const std::string& code);
    std::vector<Node> find_parens(const std::string& code);
    std::vector<Node> find_arrays(const std::string& code);
    std::vector<Node> find_objects(const std::string& code);

    bool try_remove(const std::string& code,
                    const Node& n,
                    std::function<bool(const std::string&)> test_crash,
                    std::string& out);

    bool try_replace_literal(const std::string& code,
                             const Node& n,
                             std::function<bool(const std::string&)> test_crash,
                             std::string& out);
};
