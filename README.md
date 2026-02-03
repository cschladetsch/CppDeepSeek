# CppDeepSeek

High-performance C++20 client for DeepSeek-R1 with reasoning separation, streaming, and a reusable `ModelStore` library for shared model locations.

**Build**
```bash
cmake -S . -B build
cmake --build build
```

**Install deps (Ubuntu/Debian)**
```bash
scripts/install_deps.sh
```

**Build app (default ON)**
```bash
cmake -S . -B build -DCPPDEEPSEEK_BUILD_APP=ON
cmake --build build
```

**Run (local-only default)**
```bash
./build/CppDeepSeek
```

**Run with DeepSeek API**
```bash
DEEPSEEK_API_KEY=your_key ./build/CppDeepSeek --remote
```

**CLI examples**
```bash
./build/CppDeepSeek --topic "Is C++ a good agent runtime?" --rounds 2
DEEPSEEK_API_KEY=your_key ./build/CppDeepSeek --remote --model deepseek-reasoner --no-stream
./build/CppDeepSeek --load agent_memory.json --save agent_memory.json
```

**Environment**
- `DEEPSEEK_API_KEY`: API key for DeepSeek.
- `DEEPSEEK_MODEL_HOME`: Optional override for the global model store.
- `XDG_DATA_HOME`: Optional base for the default model store.

Default model store:
- `~/.local/share/deepseek/models`

**ModelStore (separate library)**
`modelstore/` is a standalone library that can be split into its own repo.  
See `modelstore/README.md` for build, test, and install details.

**Ensure models via CMake**
See `modelstore/README.md`.

**Install ModelStore**
```bash
cmake -S modelstore -B build/modelstore -DBUILD_SHARED_LIBS=ON
cmake --build build/modelstore
cmake --install build/modelstore
```

Then in another project:
```cmake
find_package(ModelStore CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE ModelStore::ModelStore)
```
