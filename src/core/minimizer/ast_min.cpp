#include "ast_min.h"
#include <stack>

std::vector<AstMin::Node> AstMin::find_blocks(const std::string& code) {
    std::vector<Node> out;
    std::stack<size_t> st;

    for (size_t i = 0; i < code.size(); ++i) {
        if (code[i] == '{') st.push(i);
        else if (code[i] == '}' && !st.empty()) {
            out.push_back({st.top(), i});
            st.pop();
        }
    }
    return out;
}

std::vector<AstMin::Node> AstMin::find_parens(const std::string& code) {
    std::vector<Node> out;
    std::stack<size_t> st;

    for (size_t i = 0; i < code.size(); ++i) {
        if (code[i] == '(') st.push(i);
        else if (code[i] == ')' && !st.empty()) {
            out.push_back({st.top(), i});
            st.pop();
        }
    }
    return out;
}

std::vector<AstMin::Node> AstMin::find_arrays(const std::string& code) {
    std::vector<Node> out;
    std::stack<size_t> st;

    for (size_t i = 0; i < code.size(); ++i) {
        if (code[i] == '[') st.push(i);
        else if (code[i] == ']' && !st.empty()) {
            out.push_back({st.top(), i});
            st.pop();
        }
    }
    return out;
}

std::vector<AstMin::Node> AstMin::find_objects(const std::string& code) {
    // объекты совпадают с блоками, но мы можем использовать тот же метод
    return find_blocks(code);
}

bool AstMin::try_remove(const std::string& code,
                        const Node& n,
                        std::function<bool(const std::string&)> test_crash,
                        std::string& out)
{
    std::string candidate = code.substr(0, n.start) + code.substr(n.end + 1);

    if (test_crash(candidate)) {
        out = candidate;
        return true;
    }
    return false;
}

bool AstMin::try_replace_literal(const std::string& code,
                                 const Node& n,
                                 std::function<bool(const std::string&)> test_crash,
                                 std::string& out)
{
    std::string candidate =
        code.substr(0, n.start) + "0" + code.substr(n.end + 1);

    if (test_crash(candidate)) {
        out = candidate;
        return true;
    }
    return false;
}

std::string AstMin::minimize(const std::string& code,
                             std::function<bool(const std::string&)> test_crash)
{
    std::string current = code;

    auto nodes = find_blocks(current);
    auto parens = find_parens(current);
    auto arrays = find_arrays(current);
    auto objects = find_objects(current);

    std::vector<Node> all;
    all.insert(all.end(), nodes.begin(), nodes.end());
    all.insert(all.end(), parens.begin(), parens.end());
    all.insert(all.end(), arrays.begin(), arrays.end());
    all.insert(all.end(), objects.begin(), objects.end());

    bool changed = true;

    while (changed) {
        changed = false;

        for (auto& n : all) {
            std::string out;

            if (try_remove(current, n, test_crash, out)) {
                current = out;
                changed = true;
                break;
            }

            if (try_replace_literal(current, n, test_crash, out)) {
                current = out;
                changed = true;
                break;
            }
        }
    }

    return current;
}
