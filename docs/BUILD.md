# Build Instructions

## Prerequisites

### Required Software

- **C++17** or later compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake 3.20+**
- **vcpkg** (for dependency management)
- **Protobuf compiler** (protoc)
- **gRPC C++ plugin**
- **Python 3.8+** (for scripts, optional)

### System Dependencies

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    libcurl4-openssl-dev \
    libsqlite3-dev \
    pkg-config \
    python3 \
    python3-pip
```

#### macOS
```bash
brew install cmake git curl sqlite3 pkg-config python3
```

#### Windows
- Install Visual Studio 2019 or later with C++ support
- Install CMake from https://cmake.org/
- Install vcpkg from https://github.com/Microsoft/vcpkg

## Step-by-Step Build

### 1. Clone Repository

```bash
git clone <your-repo-url>
cd R.A.G.QuantTradingAgentProject
```

### 2. Install vcpkg

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # On Windows: bootstrap-vcpkg.bat
cd ..
```

### 3. Install Dependencies via vcpkg

```bash
./vcpkg/vcpkg install \
    curl \
    nlohmann-json \
    sqlite3 \
    grpc \
    protobuf \
    abseil
```

### 4. Install FAISS

FAISS needs to be installed separately:

#### Option A: Build from Source

```bash
git clone https://github.com/facebookresearch/faiss.git
cd faiss
cmake -B build -DFAISS_ENABLE_GPU=OFF -DCMAKE_BUILD_TYPE=Release .
cmake --build build -j
sudo cmake --install build
```

#### Option B: Use Conda (Linux/macOS)

```bash
conda install -c pytorch faiss-cpu
```

### 5. Set Environment Variables

```bash
export ALPHA_VANTAGE_API_KEY="your_key"
export OPENAI_API_KEY="your_key"
```

Or create a `.env` file:

```bash
cp .env.example .env
# Edit .env with your API keys
```

### 6. Build Project

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### 7. Run Server

```bash
./rag_agent_server
```

## Using the Setup Script

Alternatively, use the provided setup script:

```bash
chmod +x setup.sh
./setup.sh
```

## Troubleshooting

### CMake Cannot Find Dependencies

If CMake cannot find vcpkg packages:

1. Ensure vcpkg is properly installed
2. Use the correct toolchain file path:
   ```bash
   cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
   ```

### FAISS Not Found

If FAISS is not found:

1. Install FAISS (see above)
2. Set `FAISS_ROOT` environment variable:
   ```bash
   export FAISS_ROOT=/path/to/faiss
   ```
3. Or modify CMakeLists.txt to point to FAISS installation

### Protobuf Generation Fails

If protobuf generation fails:

1. Ensure `protoc` is in PATH:
   ```bash
   which protoc
   ```
2. Install protobuf compiler:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install protobuf-compiler
   
   # macOS
   brew install protobuf
   ```
3. Ensure gRPC C++ plugin is available:
   ```bash
   which grpc_cpp_plugin
   ```

### gRPC Build Issues

If gRPC build fails:

1. Ensure all dependencies are installed via vcpkg
2. Try building gRPC separately:
   ```bash
   ./vcpkg/vcpkg install grpc --feature-flags=versioning
   ```

### Python Bindings Not Building

If Python bindings fail to build:

1. Install pybind11:
   ```bash
   pip install pybind11
   ```
2. Or disable Python bindings:
   ```bash
   cmake .. -DBUILD_PYTHON_BINDINGS=OFF
   ```

## Docker Build

### Build Docker Image

```bash
docker build -t rag-quant-trading-agent .
```

### Run with Docker Compose

```bash
docker-compose up -d
```

### Environment Variables in Docker

Create a `.env` file:

```env
ALPHA_VANTAGE_API_KEY=your_key
OPENAI_API_KEY=your_key
```

Then run:

```bash
docker-compose up -d
```

## Development Build

For development with debugging symbols:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Debug
```

## Cross-Platform Building

### Windows

1. Open Developer Command Prompt for VS
2. Run CMake:
   ```cmd
   mkdir build
   cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
   cmake --build . --config Release
   ```

### Linux (from Windows with WSL)

1. Install WSL2
2. Follow Linux build instructions
3. Use X11 forwarding for GUI applications

## Continuous Integration

### GitHub Actions

See `.github/workflows/ci.yml` for CI configuration.

### Travis CI

See `.travis.yml` for Travis configuration.

## Performance Optimization

### Release Build

Always use Release build for production:

```bash
cmake --build . --config Release
```

### Optimization Flags

Add optimization flags in CMakeLists.txt:

```cmake
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
```

## Verification

After building, verify the installation:

```bash
# Check executable exists
ls -la build/rag_agent_server

# Check shared libraries
ldd build/rag_agent_server  # Linux
otool -L build/rag_agent_server  # macOS
```

## Next Steps

After successful build:

1. Run the server: `./build/rag_agent_server`
2. Test with client: `python3 scripts/client_example.py`
3. Ingest data: `python3 scripts/ingest_data.py --symbols AAPL --days 30`
4. Build vector index: `python3 scripts/build_vector_index.py`

