#include "core/generator/generator.h"
#include "core/pattern/pattern_analyzer.h"
#include "persistence/corpus_store.h"
#include "bus/event_bus.h"

#include <random>

namespace {

// простой RNG для выбора вариантов
std::mt19937& rng() {
    static std::mt19937 gen{std::random_device{}()};
    return gen;
}

int rand_int(int lo, int hi) {
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(rng());
}

bool rand_bool(double p = 0.5) {
    std::bernoulli_distribution dist(p);
    return dist(rng());
}

void append_safe(std::string& code, const std::string& snippet) {
    code += "\n" + snippet + "\n";
}

// -----------------------------
// Опасные паттерны
// -----------------------------

void append_wasm_oob(std::string& code) {
    // минимальный, но уже опасный wasm-модуль с memory и потенциальным OOB
    append_safe(code,
        "const wasmBytes = new Uint8Array([\n"
        "  0x00,0x61,0x73,0x6d, // \\0asm\n"
        "  0x01,0x00,0x00,0x00, // version\n"
        "  0x05,0x03,0x01,0x00,0x01, // memory section\n"
        "  0x0a,0x09,0x01,0x07,0x00,\n"
        "  0x41,0x80,0x80,0x80,0x00, // i32.const (large index)\n"
        "  0x28,0x02,0x00,           // i32.load\n"
        "  0x0b\n"
        "]);\n"
        "WebAssembly.instantiate(wasmBytes).then(r => {\n"
        "  try { r.instance.exports.main && r.instance.exports.main(); } catch (e) {}\n"
        "}).catch(() => {});\n"
    );
}

void append_typedarray_oob(std::string& code) {
    append_safe(code,
        "let buf = new ArrayBuffer(32);\n"
        "let ta = new Uint8Array(buf);\n"
        "let dv = new DataView(buf);\n"
        "try {\n"
        "  ta[100000000] = 1;\n"
        "  ta[-1] = 2;\n"
        "  ta[NaN] = 3;\n"
        "  dv.setUint32(999999999, 0x41414141);\n"
        "  dv.getFloat64(-8);\n"
        "} catch (e) {}\n"
    );
}

void append_proxy_recursion(std::string& code) {
    append_safe(code,
        "const target = {};\n"
        "const handler = {\n"
        "  get(obj, prop, recv) {\n"
        "    try { return recv[prop]; } catch (e) { return 42; }\n"
        "  },\n"
        "  set(obj, prop, value, recv) {\n"
        "    try { recv[prop] = value; } catch (e) {}\n"
        "    return true;\n"
        "  },\n"
        "  has(obj, prop) {\n"
        "    try { return prop in new Proxy(obj, handler); } catch (e) { return false; }\n"
        "  }\n"
        "};\n"
        "const p = new Proxy(target, handler);\n"
        "try {\n"
        "  for (let i = 0; i < 1000; ++i) {\n"
        "    p['x' + i] = i;\n"
        "    void p['y' + i];\n"
        "    'z' in p;\n"
        "  }\n"
        "} catch (e) {}\n"
    );
}

void append_regexp_backtracking(std::string& code) {
    append_safe(code,
        "const re = /(a+)+$/;\n"
        "const s = 'a'.repeat(5000) + 'b';\n"
        "try { re.test(s); } catch (e) {}\n"
    );
}

void append_jit_deopt_storm(std::string& code) {
    append_safe(code,
        "function hot(x) {\n"
        "  let obj = {};\n"
        "  for (let i = 0; i < 1000; ++i) {\n"
        "    obj['k' + i] = i;\n"
        "  }\n"
        "  if (typeof x === 'number') return x + 1;\n"
        "  if (typeof x === 'string') return x + 'x';\n"
        "  return x;\n"
        "}\n"
        "for (let i = 0; i < 1000; ++i) hot(i);\n"
        "for (let i = 0; i < 1000; ++i) hot('' + i);\n"
        "for (let i = 0; i < 1000; ++i) hot({ v: i });\n"
    );
}

void append_gc_pressure(std::string& code) {
    append_safe(code,
        "let big = [];\n"
        "for (let i = 0; i < 10000; ++i) {\n"
        "  big.push(new Array(1000).fill(i));\n"
        "}\n"
        "big = null;\n"
        "if (globalThis.gc) gc();\n"
    );
}

} // namespace

Generator::Generator(EventBus& bus,
                     GpuBias* gpu_bias,
                     CorpusStore* corpus)
    : bus_(bus)
    , gpu_bias_(gpu_bias)
    , corpus_(corpus)
    , arena_(1024 * 1024)
    , mutator_(arena_)
{}

void Generator::reset() {
    arena_.reset();
}

std::string Generator::build_seed() {
    if (corpus_ && corpus_->size() > 0)
        return corpus_->get_random();

    return
        "function main() {\n"
        "  let x = 0;\n"
        "  for (let i = 0; i < 1000; ++i) {\n"
        "    x += i;\n"
        "  }\n"
        "  return x;\n"
        "}\n"
        "main();\n";
}

void Generator::apply_gpu_bias(std::string& code,
                               const GpuBiasResult& bias)
{
    if (bias.total_bias > 0.2f)
        append_safe(code, "let y = Math.random();");

    if (bias.total_bias > 0.4f)
        append_safe(code, "let z = y ** 3;");

    if (bias.total_bias > 0.5f)
        append_safe(code, "new Proxy({}, { get: () => 1337 });");

    if (bias.total_bias > 0.6f)
        append_safe(code,
            "Atomics.add(new Int32Array(new SharedArrayBuffer(4)), 0, 1);");

    if (bias.total_bias > 0.7f) {
        // вместо минимального wasm — уже опасный
        append_wasm_oob(code);
    }

    if (bias.total_bias > 0.8f) {
        append_typedarray_oob(code);
    }

    if (bias.total_bias > 0.9f) {
        append_proxy_recursion(code);
        append_regexp_backtracking(code);
    }

    bus_.publish("generator:gpu_bias", "bias=" + std::to_string(bias.total_bias));
}

void Generator::apply_pattern_bias(std::string& code,
                                   const std::vector<PatternHit>& patterns)
{
    float accum = 0.0f;
    for (const auto& p : patterns)
        accum += p.weight;

    if (accum > 0.3f)
        append_safe(code, "let arr = new Uint8Array(64);");

    if (accum > 0.6f)
        append_safe(code, "let dv = new DataView(new ArrayBuffer(64));");

    if (accum > 0.7f && rand_bool(0.5)) {
        append_typedarray_oob(code);
    }

    if (accum > 0.8f && rand_bool(0.5)) {
        append_proxy_recursion(code);
    }

    if (accum > 0.9f) {
        append_jit_deopt_storm(code);
        append_gc_pressure(code);
    }

    bus_.publish("generator:pattern_bias", "accum=" + std::to_string(accum));
}

Script Generator::generate() {
    bus_.publish("generator:start", "generator started");

    arena_.reset();

    Script s;
    s.code = build_seed();

    // 1) первичная мутация
    s.code = mutator_.mutate(s.code);

    // 2) анализ паттернов
    std::vector<PatternHit> patterns = analyzer_.analyze(s.code);

    // 3) GPU bias
    if (gpu_bias_) {
        GpuBiasResult bias = gpu_bias_->compute(s.code, patterns);
        apply_gpu_bias(s.code, bias);
    }

    // 4) pattern bias
    apply_pattern_bias(s.code, patterns);

    // 5) финальная мутация
    s.code = mutator_.mutate(s.code);

    bus_.publish("generator:new_script", s.code);
    bus_.publish("generator:end", "generator finished");

    return s;
}

void Generator::update_bias(const std::vector<PatternHit>& patterns) {
    bus_.publish("generator:update_bias",
                 "patterns=" + std::to_string(patterns.size()));
}
