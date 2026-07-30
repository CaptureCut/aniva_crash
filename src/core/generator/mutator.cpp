#include "core/generator/mutator.h"

Mutator::Mutator() {
    std::random_device rd;
    rng_ = std::mt19937(rd());
}

Mutator::Mutator(Arena& arena)
    : arena_(&arena)
{
    std::random_device rd;
    rng_ = std::mt19937(rd());
}

std::string Mutator::mutate(const std::string& code) {
    if (code.empty())
        return code;

    std::uniform_int_distribution<int> dist(0, 2);
    int choice = dist(rng_);

    switch (choice) {
        case 0:
            return code + "\n" + insert_random_stmt();

        case 1:
            return flip_operator(code);

        case 2:
            return code + "\n" + random_identifier() + " = " + random_identifier() + "();";
    }

    return code;
}

std::string Mutator::insert_random_stmt() {
    static const std::vector<std::string> stmts = {
        "let a = Math.random();",
        "let b = new Array(10);",
        "let c = {x: 1, y: 2};",
        "function f() { return 42; }",
        "try { throw 1; } catch(e) {}",
        "for (let i = 0; i < 5; i++) {}",
        "let z = ({}).__proto__;"
    };

    std::uniform_int_distribution<int> dist(0, stmts.size() - 1);
    return stmts[dist(rng_)];
}

std::string Mutator::flip_operator(const std::string& code) {
    std::string out = code;

    static const std::vector<std::pair<std::string, std::string>> ops = {
        {"+", "-"},
        {"-", "+"},
        {"*", "/"},
        {"/", "*"},
        {"==", "!="},
        {"!=", "=="},
        {"<", ">"},
        {">", "<"}
    };

    std::uniform_int_distribution<int> dist(0, ops.size() - 1);
    auto op = ops[dist(rng_)];

    size_t pos = out.find(op.first);
    if (pos != std::string::npos) {
        out.replace(pos, op.first.size(), op.second);
    }

    return out;
}

std::string Mutator::random_identifier() {
    static const std::vector<std::string> ids = {
        "foo", "bar", "baz", "qux", "alpha", "beta", "gamma"
    };

    std::uniform_int_distribution<int> dist(0, ids.size() - 1);
    return ids[dist(rng_)];
}
