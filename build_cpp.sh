#!/usr/bin/env bash
set -eo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CPP_SRC_DIR="$PROJECT_ROOT/Native_CPP_build"
OUT_DIR="$PROJECT_ROOT/bin/Debug/net10.0"

# Ensure output directory exists
mkdir -p "$OUT_DIR"

if command -v cmake >/dev/null 2>&1; then
    echo -e "\e[1;34m[*] CMake found. Building C++ engine via CMake...\e[0m"
    BUILD_DIR="$CPP_SRC_DIR/build"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make -j"$(nproc)"
else
    echo -e "\e[1;33m[!] CMake not found. Falling back to direct g++ umbrella compilation...\e[0m"

    if ! command -v g++ >/dev/null 2>&1; then
        echo -e "\e[1;31m[-] Error: Neither cmake nor g++ was found in PATH.\e[0m"
        exit 1
    fi

    cd "$CPP_SRC_DIR"

    # Aggressive optimizations applied globally for the unified engine
    CXX_FLAGS="-shared -fPIC -std=c++17 -O3 -march=native -Wall"
    SSL_LIBS="-lssl -lcrypto"

    echo "  -> Compiling reconsage_native.so (Unified Engine)..."
    
    g++ $CXX_FLAGS -o "$OUT_DIR/reconsage_native.so" \
        parser.cpp rso_parser.cpp rxo_parser.cpp \
        ScanModule/scan.cpp \
        TorScan/tor_scan.cpp \
        Response_Body_capture/res_body_captio.cpp \
        Reco_GAN/Reco_GAN_Training.cpp Reco_GAN/Reco_GAN_Module.cpp \
        Reco_GAN/Reco_GAN_Adv_Trees/Training.cpp Reco_GAN/Reco_GAN_Adv_Trees/Predict.cpp \
        Reco_GAN/Reco_GAN_Adv_Trees/Waf_Catcher.cpp \
        Proxy_Scanning/HttpProxyScanModule.cpp Proxy_Scanning/SockProxy_module.cpp \
        Compiler/compiler.cpp \
        $SSL_LIBS
fi

echo -e "\e[1;32m[✔] Native engine successfully compiled into: $OUT_DIR/reconsage_native.so\e[0m"