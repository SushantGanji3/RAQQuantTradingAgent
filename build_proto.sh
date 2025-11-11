#!/bin/bash

# Script to generate protobuf and gRPC code

set -e

echo "🔨 Generating protobuf and gRPC code..."

# Check for protoc
command -v protoc >/dev/null 2>&1 || { echo "❌ protoc is required but not installed. Abort." >&2; exit 1; }

# Check for grpc_cpp_plugin
command -v grpc_cpp_plugin >/dev/null 2>&1 || { echo "❌ grpc_cpp_plugin is required but not installed. Abort." >&2; exit 1; }

# Create output directory
mkdir -p build/generated

# Generate protobuf and gRPC code
protoc --cpp_out=build/generated \
       --grpc_out=build/generated \
       --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` \
       --proto_path=proto \
       proto/rag_service.proto

echo "✅ Protobuf and gRPC code generated successfully!"
echo "   Output directory: build/generated/"

