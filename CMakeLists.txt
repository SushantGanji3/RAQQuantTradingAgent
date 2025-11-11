cmake_minimum_required(VERSION 3.20)
project(RAGQuantTradingAgent VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)

# Find packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# C++ dependencies
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# SQLite3
find_library(SQLITE3_LIB sqlite3)
find_path(SQLITE3_INCLUDE sqlite3.h)

# DuckDB (optional)
find_package(duckdb CONFIG QUIET)

# gRPC
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Abseil (optional, often included with gRPC)
find_package(abseil QUIET)
if(NOT abseil_FOUND)
    message(STATUS "Abseil not found, continuing without it")
endif()

# FAISS (optional - can be installed separately)
find_path(FAISS_INCLUDE faiss/Index.h QUIET)
find_library(FAISS_LIB faiss QUIET)

# Check if FAISS is available
if(NOT FAISS_INCLUDE OR NOT FAISS_LIB OR FAISS_INCLUDE MATCHES "NOTFOUND" OR FAISS_LIB MATCHES "NOTFOUND")
    message(WARNING "FAISS not found. Building without FAISS support. Install FAISS for vector search functionality.")
    add_definitions(-DNO_FAISS)
    set(FAISS_AVAILABLE FALSE)
else()
    set(FAISS_AVAILABLE TRUE)
    message(STATUS "FAISS found: ${FAISS_INCLUDE}")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLITE3_INCLUDE}
    ${CURL_INCLUDE_DIRS}
)

# Add FAISS include if found
if(FAISS_AVAILABLE)
    include_directories(${FAISS_INCLUDE})
endif()

# Source files
set(SOURCES
    src/data_ingestion/data_fetcher.cpp
    src/data_ingestion/database.cpp
    src/vectorization/embedding_service.cpp
    src/vectorization/faiss_index.cpp
    src/rag/rag_agent.cpp
    src/api/grpc_server.cpp
    src/utils/logger.cpp
)

# Protobuf files
set(PROTO_FILES
    proto/rag_service.proto
)

# Generate protobuf and gRPC code
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

add_custom_command(
    OUTPUT ${PROTO_GEN_DIR}/rag_service.pb.cc
           ${PROTO_GEN_DIR}/rag_service.pb.h
           ${PROTO_GEN_DIR}/rag_service.grpc.pb.cc
           ${PROTO_GEN_DIR}/rag_service.grpc.pb.h
    COMMAND protobuf::protoc
    ARGS --cpp_out=${PROTO_GEN_DIR}
         --grpc_out=${PROTO_GEN_DIR}
         --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
         -I ${CMAKE_CURRENT_SOURCE_DIR}/proto
         ${CMAKE_CURRENT_SOURCE_DIR}/proto/rag_service.proto
    DEPENDS ${PROTO_FILES}
    COMMENT "Generating protobuf and gRPC code"
)

# Add generated directory to include paths
include_directories(${PROTO_GEN_DIR})

# Main library
add_library(rag_agent_lib
    ${SOURCES}
    ${PROTO_GEN_DIR}/rag_service.pb.cc
    ${PROTO_GEN_DIR}/rag_service.grpc.pb.cc
)

target_link_libraries(rag_agent_lib
    PUBLIC
        ${CURL_LIBRARIES}
        nlohmann_json::nlohmann_json
        ${SQLITE3_LIB}
        gRPC::grpc++
        protobuf::libprotobuf
        Threads::Threads
)

# Add Abseil if found
if(abseil_FOUND)
    target_link_libraries(rag_agent_lib PUBLIC absl::base absl::strings)
endif()

# Optional dependencies
if(duckdb_FOUND)
    target_link_libraries(rag_agent_lib PUBLIC duckdb::duckdb)
endif()

if(FAISS_AVAILABLE)
    target_link_libraries(rag_agent_lib PUBLIC ${FAISS_LIB})
endif()

# Main executable
add_executable(rag_agent_server
    src/main.cpp
)

target_link_libraries(rag_agent_server
    rag_agent_lib
)

# Python bindings (optional)
if(BUILD_PYTHON_BINDINGS)
    find_package(pybind11 QUIET)
    if(pybind11_FOUND)
        pybind11_add_module(rag_agent_py
            src/python_bindings/python_bridge.cpp
        )
        target_link_libraries(rag_agent_py PRIVATE rag_agent_lib)
    endif()
endif()

# Installation
install(TARGETS rag_agent_server rag_agent_lib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
