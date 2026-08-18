#!/usr/bin/env bash
set -eo pipefail

echo -e "\e[1;36m================================================\e[0m"
echo -e "\e[1;36m       ReconSage CLI — Full Build Pipeline      \e[0m"
echo -e "\e[1;36m================================================\e[0m"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_ROOT"

chmod +x build_csharp.sh build_cpp.sh

# 1. Build C# Shell
./build_csharp.sh

# 2. Build C++ Native Shared Objects
./build_cpp.sh

echo ""
echo -e "\e[1;32m[✔] Full pipeline built successfully.\e[0m"
echo -e "\e[1;37mRun the shell anytime with:\e[0m \e[1;33mdotnet run\e[0m"