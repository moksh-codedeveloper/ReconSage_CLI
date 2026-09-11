# ReconSage CLI

**ReconSage 1.0** is an advanced behavioral reconnaissance scanner, telemetry pipeline, and predictive intelligence framework built from the ground up in **C++ and C#**.

Designed as the V12 engine of network reconnaissance, ReconSage bypasses standard HTTP client abstractions to interact directly with the metal via raw POSIX syscalls (`socket()`, `connect()`, `send()`, `recv()`). It features a completely custom, byte-level DNS stack (dropping libc `getaddrinfo`), strictly TCP-bound Tor routing to preserve latency math, and native Machine Learning models (GANs and from-scratch Isolation Forest decision trees) for real-time WAF anomaly detection.

> **The Origins of ReconSage:** ReconSage began years ago as an experimental Python script. ReconSage 1.0 represents a complete, ground-up architectural rewrite into a unified C++ core and C# orchestrator to deliver untamed, zero-bottleneck performance and dynamically compiled ML reconnaissance.

---

## Interactive Shell Experience

ReconSage features an interactive Metasploit-inspired orchestration shell (`RecoShell`) designed for real-time telemetry diagnostics and fast command execution:

- **Dynamic Context Prompts:** Automatically binds to your active target once loaded (`reconsage (target.com) > `).
- **Runtime Telemetry Status:** Live color-coded indicators tracking loaded configurations (`RSO:LOADED | RFO:LOADED | RXO:LOADED`).
- **Seamless OS Shell Passthrough:** Execute system/network commands inline using the `!` prefix or drop into an interactive subshell session (`zsh`/`bash`) without terminating the current recon context.
- **Categorized Command Dispatcher:** Clean multi-file modular command routing with built-in help and diagnostics.

```text
    __________  ________________  _____  ___ __________
    ___  __ \_  / __  __ \_  __ \__  / / /_  ____/_  _ \
    __  /_/ /  /  _  / / /  / / /_  /_/ /_  / __ _  / / /
    _  _, _// /___/ /_/ // /_/ /_  __  / / /_/ / / /_/ /
    /_/ |_|/_____/\____/ /____/ /_/ /_/  \____/  \____/
                                         v1.0.0 [ARCH-LINUX]
--------------------------------------------------------------
=[ ReconSage | Advanced Telemetry Framework ]=
+ -- --=[ Status: RSO:LOADED | RFO:LOADED | RXO:LOADED ]=
--------------------------------------------------------------

reconsage (www.google.com) >

```

---

## Architecture Overview

ReconSage is designed as a hyper-optimized, two-tier system communicating via high-performance **P/Invoke** interop:

1. **C# Shell & Orchestration (`ReconSageShell`)**:

- Modular async CLI shell managing scan lifecycles, configuration sessions, cancellation tokens, and database staging.
- Encapsulated parser interfaces (`IFileParser<T>`) for clean config lifecycle management.

2. **C++ Native Umbrella Engine (`reconsage_native.so`)**:

- A single, monolithic compiled engine housing all scanning, DNS, and ML logic for zero-bottleneck memory sharing.
- Native ML pipelines running custom Isolation Forest algorithms and Deterministic WAF Catchers natively in memory.

```text
reconsage (target.com) > (C# Shell / SessionData Orchestration)
      │
      ├── Configuration Ingestion (.rfo / .rso / .rxo via IFileParser<T>)
      ├── Host OS Shell & Passthrough Integration (!cmd, exec, shell)
      │
      └── Unified Native Engine (P/Invoke -> reconsage_native.so):
            ├── POSIX Scan Engine       → Direct TCP/TLS probing with OpenSSL
            ├── Tor Routing Engine      → SOCKS5 + Tor Control Circuit Rotation (TCP ONLY)
            ├── Proxy Engine            → HTTP & SOCKS proxy routing
            ├── Custom DNS Stack        → Wire-level resolver (bypasses OS locks)
            ├── Capture Engine          → Raw HTML/payload and header stream extraction
            ├── Database Compiler       → JSON-to-DB indexing, status code & latency filters
            └── ML & Heuristics Core:
                  ├── Reco_GAN          → Generative path prediction
                  ├── iForest Trees     → Custom C++ Isolation Forest latency analysis
                  └── WAF Catcher       → Tarpit / Soft-404 deterministic correlation

```

---

## Configuration Files

ReconSage uses three dedicated configuration formats to control scans, targets, and ML/DB execution:

### 1. `.rfo` — Recon Target Options

Defines target network parameters, custom DNS resolvers, and proxy/Tor authentication.

```ini
[target]
target      = example.com
proto_port  = 443
dns_server  = 1.1.1.1
tor_ip      = 127.0.0.1
tor_port    = 9050
port        = 9051
password    = your_tor_control_password

```

### 2. `.rso` — Recon Scan Options

Controls scan timings, jitter/delays, wordlists, and capture targets.

```ini
[scan]
timeout         = 5000
delay           = 100
wordlist_path   = /path/to/wordlist.txt
json_file_path  = /path/to/output.json
headers_file    = /path/to/headers.txt
html_file       = /path/to/captured_body.html

```

### 3. `.rxo` — Extended DB & ML Options

Controls database credentials, filtering criteria, and machine learning hyperparameters.

```ini
[extended]
db_password     = your_db_password
status_code     = 200
latency         = 300.56
k_factor        = 5
num_trees       = 100
sub_sample_size = 256

```

---

## Shell Command Reference

### Core & Navigation

| Command  | Arguments | Description                                                   |
| -------- | --------- | ------------------------------------------------------------- |
| `help`   | None      | Displays the categorized command menu and descriptions        |
| `banner` | None      | Redraws the startup header with live loaded-profile telemetry |
| `clear`  | None      | Clears the active console screen                              |
| `exit`   | None      | Terminates active sessions and shuts down the framework       |

### OS Shell & Subsystem Passthrough

| Command          | Arguments | Description                                                        |
| ---------------- | --------- | ------------------------------------------------------------------ |
| `!<command>`     | `<cmd>`   | Instant execution of a host command (e.g., `!nmap -sV`, `!ls -lh`) |
| `exec <command>` | `<cmd>`   | Explicitly invokes a shell command via your default `$SHELL`       |
| `shell`          | None      | Drops into an interactive system subshell (type `exit` to return)  |

### Scanning Modules

| Command                  | Dependencies | Description                                                         |
| ------------------------ | ------------ | ------------------------------------------------------------------- |
| `start_scan_cpp`         | RFO, RSO     | Executes direct POSIX socket scan with OpenSSL TLS and Custom DNS   |
| `start_tor_scan`         | RFO, RSO     | Executes anonymous scan over Tor SOCKS5 with auto-circuit switching |
| `start_http_proxy_scan`  | RFO, RSO     | Routes custom HTTP probing requests through an upstream HTTP proxy  |
| `start_socks_proxy_scan` | RFO, RSO     | Tunnels scan through an arbitrary SOCKS proxy                       |
| `start_cpp_body_capture` | RFO, RSO     | Captures raw response HTML bodies and headers directly to disk      |

### Native ML & Heuristic Engine

| Command                  | Dependencies | Description                                                                   |
| ------------------------ | ------------ | ----------------------------------------------------------------------------- |
| `reco_gan_training`      | RFO, RXO     | Trains the native GAN model on target behavioral data using `k_factor`        |
| `reco_gan_predict`       | RFO, RXO     | Runs generative path and behavior predictions on trained weights              |
| `reco_gan_trees_train`   | RFO, RXO     | Trains native C++ Isolation Forest trees to detect anomalous latencies        |
| `reco_gan_trees_predict` | RFO, RXO     | Evaluates target behavior against compiled trees for anomaly scoring          |
| `reco_gan_catch_waf`     | RFO, RXO     | Deterministic WAF catcher combining structural analysis and latency profiling |

---

## Installation & Compilation

ReconSage is optimized for Arch Linux but supports any POSIX-compliant environment with .NET 10.0+.

### Arch Linux (AUR)

The easiest way to install the stable release on Arch-based systems:

```bash
yay -S reconsage

```

### Manual Build From Source

The native core requires **OpenSSL** development headers and a C++17 compliant compiler.

**1. Compile the C++ Engine (CMake Recommended):**

```bash
cd Native_CPP_build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

```

**2. Compile the C# Orchestrator:**

```bash
cd ..
dotnet build -c Release

```

Ensure the compiled `reconsage_native.so` library is present in your binary output directory (e.g., `bin/Debug/net10.0/`), then launch the orchestrator:

```bash
dotnet run

```

---

## System Requirements

- **Operating System:** Linux (POSIX-compliant; optimized for Arch Linux)
- **Runtime:** .NET 10.0 SDK
- **Compiler:** `cmake` (3.20+) or `g++` (C++17 standard)
- **Libraries:** OpenSSL (`libssl-dev` / `openssl`)
- **Proxy Services (Optional):** Active Tor daemon on port `9050` with Control Port enabled on `9051`

---

## License

This project is licensed under the **GNU General Public License v3.0 (GPL-3.0)**. See the `LICENSE` file in the root repository for the complete text.

---

## Authorized Testing & Legal Notice

> **Authorized Use Only:** ReconSage is built strictly for authorized network auditing, defensive security engineering, vulnerability assessment, and academic research. Conducting scans or capturing payloads against systems without explicit, documented permission is strictly prohibited by law. The authors and maintainers assume no liability for misuse.
