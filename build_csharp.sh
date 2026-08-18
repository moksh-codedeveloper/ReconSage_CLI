#!/usr/bin/env bash
set -eo pipefail

echo -e "\e[1;34m[*] Building C# Shell Orchestrator (.NET)...\e[0m"

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_ROOT"

if ! command -v dotnet >/dev/null 2>&1; then
    echo -e "\e[1;31m[-] Error: dotnet SDK is not installed or not in PATH.\e[0m"
    exit 1
fi

dotnet build

echo -e "\e[1;32m[✔] C# build complete.\e[0m"