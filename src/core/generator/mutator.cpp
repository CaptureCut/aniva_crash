#include "core/generator/mutator.h"
#include <regex>
#include <random>

namespace {

std::mt19937& rng() {
    static std::mt19937 gen{std::random_device{}()};
    return gen;
}

int rint(int lo, int hi) {
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(rng());
}

bool rbool(double p = 0.5) {
    std::bernoulli_distribution dist(p);
    return dist(rng());
}

std::string pick(const std::vector<std::string>& v) {
    return v[rint(0, (int)v.size() - 1)];
}

} // namespace

Mutator::Mutator() {
    rng_ = std::mt19937(std::random_device{}());
}

Mutator::Mutator(Arena& arena)
    : arena_(&arena)
{
    rng_ = std::mt19937(std::random_device{}());
}

std::string Mutator::mutate(const std::string& code)
{
    if (code.empty())
        return code;

    std::uniform_int_distribution<int> dist(0, 7);
    int choice = dist(rng_);

    switch (choice) {
        case 0: return insert_safe_stmt(code);
        case 1: return flip_safe_operator(code);
        case 2: return wrap_in_try(code);
        case 3: return append_function_call(code);
        case 4: return append_wasm_trigger(code);
        case 5: return insert_oob_index(code);
        case 6: return insert_strange_value(code);
        case 7: return insert_proxy_glitch(code);
    }

    return code;
}

// ------------------------------------------------------------
// SAFE INSERTION
// ------------------------------------------------------------
std::string Mutator::insert_safe_stmt(const std::string& code)
{
    static const std::vector<std::string> stmts = {
        "let a = Math.random();",
        "let b = new Array(10);",
        "let c = {x: 1, y: 2};",
        "function f_mut() { return 42; }",
        "for (let i = 0; i < 5; i++) {}",
        "let z = ({}).__proto__;"
    };

    return code + "\n" + pick(stmts) + "\n";
}

// ------------------------------------------------------------
// SAFE OPERATOR FLIP
// ------------------------------------------------------------
std::string Mutator::flip_safe_operator(const std::string& code)
{
    std::string out = code;

    static const std::vector<std::pair<std::string, std::string>> ops = {
        {" + ", " - "},
        {" - ", " + "},
        {" * ", " / "},
        {" / ", " * "}
    };

    auto op = ops[rint(0, (int)ops.size() - 1)];

    size_t pos = out.find(op.first);
    if (pos != std::string::npos)
        out.replace(pos, op.first.size(), op.second);

    return out;
}

// ------------------------------------------------------------
// WRAP CODE IN TRY-CATCH
// ------------------------------------------------------------
std::string Mutator::wrap_in_try(const std::string& code)
{
    return "try {\n" + code + "\n} catch(e) {}\n";
}

// ------------------------------------------------------------
// APPEND SAFE FUNCTION CALL
// ------------------------------------------------------------
std::string Mutator::append_function_call(const std::string& code)
{
    static const std::vector<std::string> calls = {
        "Math.sin(0);",
        "Math.max(1,2);",
        "JSON.stringify({a:1});",
        "Array.from([1,2,3]);"
    };

    return code + "\n" + pick(calls) + "\n";
}

// ------------------------------------------------------------
// WASM TRIGGER (минимальный)
// ------------------------------------------------------------
std::string Mutator::append_wasm_trigger(const std::string& code)
{
    return code +
        "\nWebAssembly.instantiate(new Uint8Array([0,97,115,109]));\n";
}

// ------------------------------------------------------------
// OOB INDEX MUTATION (опасно)
// ------------------------------------------------------------
std::string Mutator::insert_oob_index(const std::string& code)
{
    static const std::vector<std::string> oob = {
        "let ta = new Uint8Array(16); ta[99999999] = 1;",
        "let ta = new Uint8Array(16); ta[-1] = 2;",
        "let dv = new DataView(new ArrayBuffer(32)); dv.setUint32(99999999, 0x41414141);",
        "let dv = new DataView(new ArrayBuffer(32)); dv.getFloat64(-8);"
    };

    return code + "\n" + pick(oob) + "\n";
}

// ------------------------------------------------------------
// STRANGE VALUES (NaN, Infinity, Symbol, Proxy)
// ------------------------------------------------------------
std::string Mutator::insert_strange_value(const std::string& code)
{
    static const std::vector<std::string> vals = {
        "let v = NaN;",
        "let v = Infinity;",
        "let v = Symbol('x');",
        "let v = new Proxy({}, { get: () => 1337 });"
    };

    return code + "\n" + pick(vals) + "\n";
}

// ------------------------------------------------------------
// PROXY GLITCH (рекурсивные ловушки)
// ------------------------------------------------------------
std::string Mutator::insert_proxy_glitch(const std::string& code)
{
    return code +
        "\nconst t = {};\n"
        "const h = {\n"
        "  get(o,p,r){ try { return r[p]; } catch(e){ return 42; } },\n"
        "  set(o,p,v,r){ try { r[p]=v; } catch(e){} return true; },\n"
        "  has(o,p){ try { return p in new Proxy(o,h); } catch(e){ return false; } }\n"
        "};\n"
        "const px = new Proxy(t,h);\n"
        "try { px.x = 1; void px.y; 'z' in px; } catch(e){}\n";
}
