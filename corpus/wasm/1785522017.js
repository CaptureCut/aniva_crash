try {
function main() {
  let x = 0;
  for (let i = 0; i < 1000; ++i) {
    x += i;
  }
}
main();

// gpu-bias: random
let y = Math.random();
// gpu-bias: exponent
let z = y ** 3;
// gpu-bias: proxy
new Proxy({}, { get: () => 1337 });
// gpu-bias: atomics
Atomics.add(new Int32Array(new SharedArrayBuffer(4)), 0, 1);
// gpu-bias: wasm
WebAssembly.instantiate(new Uint8Array([0,97,115,109]));
let a = Math.random();
} catch(e) {}

let y = Math.random();

let z = y ** 3;

new Proxy({}, { get: () => 1337 });

Atomics.add(new Int32Array(new SharedArrayBuffer(4)), 0, 1);

WebAssembly.instantiate(new Uint8Array([0,97,115,109]));

let arr = new Uint8Array(64);

let dv = new DataView(new ArrayBuffer(64));

WebAssembly.instantiate(new Uint8Array([0,97,115,109]));

function f_mut() { return 42; }
let y = Math.random();

let z = y ** 3;

new Proxy({}, { get: () => 1337 });

Atomics.add(new Int32Array(new SharedArrayBuffer(4)), 0, 1);

WebAssembly.instantiate(new Uint8Array([0,97,115,109]));

let arr = new Uint8Array(64);

let dv = new DataView(new ArrayBuffer(64));

for (let i = 0; i < 5; i++) {}