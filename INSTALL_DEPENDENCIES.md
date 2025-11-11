# Installing Dependencies

## macOS Installation Guide

Since you're on macOS, here's how to install the required dependencies:

### 1. Install Homebrew (if not already installed)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 2. Install CMake

```bash
brew install cmake
```

### 3. Install Required System Dependencies

```bash
brew install curl sqlite3 pkg-config
```

### 4. Install vcpkg

```bash
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
./bootstrap-vcpkg.sh

# Install dependencies
./vcpkg install curl nlohmann-json sqlite3 grpc protobuf abseil
```

### 5. Install FAISS

FAISS requires manual installation. Here are the options:

#### Option A: Using Conda (Recommended for macOS)

```bash
# Install Miniconda if you don't have it
# Download from: https://docs.conda.io/en/latest/miniconda.html

# Install FAISS
conda install -c pytorch faiss-cpu
```

#### Option B: Build from Source

```bash
# Install dependencies
brew install openblas

# Clone FAISS
git clone https://github.com/facebookresearch/faiss.git
cd faiss
cmake -B build -DFAISS_ENABLE_GPU=OFF -DCMAKE_BUILD_TYPE=Release .
cmake --build build -j
sudo cmake --install build
```

### 6. Install Python Dependencies

```bash
pip3 install grpcio grpcio-tools protobuf
```

### 7. Verify Installation

Run the verification script:

```bash
./scripts/verify_setup.sh
```

## Quick Install Script

You can also use the setup script which will attempt to install most dependencies:

```bash
chmod +x setup.sh
./setup.sh
```

## Troubleshooting

### CMake not found

```bash
brew install cmake
```

### vcpkg installation issues

Make sure you have the latest version of vcpkg and all system dependencies:

```bash
brew install curl sqlite3 pkg-config
```

### FAISS installation issues

FAISS can be tricky on macOS. Try the Conda installation first:

```bash
conda install -c pytorch faiss-cpu
```

If that doesn't work, you may need to build from source with specific flags for macOS.

### gRPC installation issues

Make sure you have the latest Xcode command line tools:

```bash
xcode-select --install
```

## Next Steps

After installing dependencies:

1. Configure API keys in `.env`
2. Build the project:
   ```bash
   mkdir -p build
   cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
   cmake --build . --config Release
   ```
3. Run the server:
   ```bash
   ./rag_agent_server
   ```

## Alternative: Use Docker

If you're having trouble with dependencies, you can use Docker:

```bash
docker build -t rag-quant-trading-agent .
docker run -p 50051:50051 \
  -e ALPHA_VANTAGE_API_KEY=your_key \
  -e OPENAI_API_KEY=your_key \
  rag-quant-trading-agent
```

