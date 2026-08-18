#!/usr/bin/env bash
set -eo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CPP_SRC_DIR="$PROJECT_ROOT/Native_CPP_build"
OUT_DIR="$PROJECT_ROOT/bin/Debug/net10.0"

# Ensure output directory exists
mkdir -p "$OUT_DIR"

if command -v cmake >/dev/null 2>&1; then
    echo -e "\e[1;34m[*] CMake found. Building C++ modules via CMake...\e[0m"
    BUILD_DIR="$CPP_SRC_DIR/build"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make -j"$(nproc)"
else
    echo -e "\e[1;33m[!] CMake not found. Falling back to direct g++ multi-target compilation...\e[0m"

    if ! command -v g++ >/dev/null 2>&1; then
        echo -e "\e[1;31m[-] Error: Neither cmake nor g++ was found in PATH.\e[0m"
        exit 1
    fi

    cd "$CPP_SRC_DIR"

    CXX_FLAGS="-shared -fPIC -std=c++17 -O3 -Wall"
    SSL_LIBS="-lssl -lcrypto"

    echo "  -> [1/8] Compiling parser_cpp_module.so..."
    g++ $CXX_FLAGS -o "$OUT_DIR/parser_cpp_module.so" parser.cpp rso_parser.cpp rxo_parser.cpp

    echo "  -> [2/8] Compiling scan_cpp_module.so..."
    g++ $CXX_FLAGS -o "$OUT_DIR/scan_cpp_module.so" ScanModule/scan.cpp $SSL_LIBS

    echo "  -> [3/8] Compiling tor_cpp_module.so..."
    g++ $CXX_FLAGS -o "$OUT_DIR/tor_cpp_module.so" TorScan/tor_scan.cpp $SSL_LIBS

    echo "  -> [4/8] Compiling res_body_scan_cpp_module.so..."
    g++ $CXX_FLAGS -o "$OUT_DIR/res_body_scan_cpp_module.so" Response_Body_capture/res_body_captio.cpp $SSL_LIBS

    echo "  -> [5/8] Compiling reco_gan_ml_module_cpp_module.so..."
    g++ $CXX_FLAGS -o "$OUT_DIR/reco_gan_ml_module_cpp_module.so" Reco_GAN/Reco_GAN_Training.cpp Reco_GAN/Reco_GAN_Module.cpp

    echo "  -> [6/8] Compiling reco_gan_lat_v2_ml_module_cpp_module.so..."
    g++ $CXX_FLAGS -o "$OUT_DIR/reco_gan_lat_v2_ml_module_cpp_module.so" Reco_GAN/Reco_GAN_V2/Training.cpp

    echo "  -> [7/8] Compiling proxy_scan_cpp_module.so..."
    g++ $CXX_FLAGS -o "$OUT_DIR/proxy_scan_cpp_module.so" Proxy_Scanning/HttpProxyScanModule.cpp Proxy_Scanning/SockProxy_module.cpp $SSL_LIBS

    echo "  -> [8/8] Compiling reco_novich_compiler_cpp_module.so..."
    g++ $CXX_FLAGS -march=native -o "$OUT_DIR/reco_novich_compiler_cpp_module.so" Compiler/compiler.cpp
fi

echo -e "\e[1;32m[✔] All native shared modules successfully compiled into: $OUT_DIR\e[0m"