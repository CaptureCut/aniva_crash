#include "core/generator/generator.h"
#include "core/pattern/pattern_analyzer.h"

Generator::Generator(EventBus& bus,
                     GpuBias* gpu_bias,
                     CorpusStore* corpus)
    : bus_(bus)
    , gpu_bias_(gpu_bias)
    , corpus_(corpus)
    , arena_(1024 * 1024)
    , mutator_(arena_)
{}

std::string Generator::build_seed() {
    if (corpus_ && corpus_->size() > 0) {
        return corpus_->get_random();
    }

    return
        "function main() {\n"
        "  let x = 0;\n"
        "  for (let i = 0; i < 1000; ++i) {\n"
        "    x += i;\n"
        "  }\n"
        "}\n"
        "main();\n";
}

void Generator::apply_gpu_bias(std::string& code,
                               const GpuBiasResult& bias)
{
    if (bias.total_bias > 0.2f)
        code += "\n// gpu-bias: random\nlet y = Math.random();";

    if (bias.total_bias > 0.4f)
        code += "\n// gpu-bias: exponent\nlet z = y ** 3;";

    if (bias.total_bias > 0.5f)
        code += "\n// gpu-bias: proxy\nnew Proxy({}, { get: () => 1337 });";

    if (bias.total_bias > 0.6f)
        code += "\n// gpu-bias: atomics\nAtomics.add(new Int32Array(new SharedArrayBuffer(4)), 0, 1);";

    if (bias.total_bias > 0.7f)
        code += "\n// gpu-bias: wasm\nWebAssembly.instantiate(new Uint8Array([0,97,115,109]));";

    bus_.publish("generator:gpu_bias", "bias=" + std::to_string(bias.total_bias));
}

void Generator::apply_pattern_bias(std::string& code,
                                   const std::vector<PatternHit>& patterns)
{
    float accum = 0.0f;

    for (const auto& p : patterns) {
        accum += p.weight;

        if (p.weight > 0.5f) {
            code += "\n// pattern-hit\n"
                    "// kind=" + std::to_string(static_cast<int>(p.kind)) +
                    " weight=" + std::to_string(p.weight) +
                    " pos=" + std::to_string(p.position);
        }
    }

    if (accum > 0.3f)
        code += "\nlet arr = new Uint8Array(64);";

    if (accum > 0.6f)
        code += "\nlet dv = new DataView(new ArrayBuffer(64));";

    bus_.publish("generator:pattern_bias", "accum=" + std::to_string(accum));
}

Script Generator::generate() {
    bus_.publish("generator:start", "generator started");

    arena_.reset();

    Script s;
    s.code = build_seed();

    // новый анализатор: без tokenizer_, без quick()
    std::vector<PatternHit> patterns = analyzer_.analyze(s.code);

    // GPU bias
    if (gpu_bias_) {
        GpuBiasResult bias = gpu_bias_->compute(s.code, patterns);
        apply_gpu_bias(s.code, bias);
    }

    // pattern bias
    apply_pattern_bias(s.code, patterns);

    // mutation
    s.code = mutator_.mutate(s.code);

    bus_.publish("generator:new_script", s.code);
    bus_.publish("generator:end", "generator finished");

    return s;
}

void Generator::update_bias(const std::vector<PatternHit>& patterns) {
    bus_.publish("generator:update_bias",
                 "patterns=" + std::to_string(patterns.size()));
}
