# ReconSage CLI

**ReconSage** is an advanced behavioral reconnaissance scanner, telemetry pipeline, and predictive intelligence framework built from the ground up in **C++ and C#**.

It bypasses standard HTTP client abstractions to interact directly with the kernel and wire via raw POSIX syscalls (`socket()`, `connect()`, `send()`, `recv()`). Beyond raw scanning, ReconSage indexes response headers, captures raw HTML bodies, ingests operational metrics into a database engine, and applies custom native Machine Learning models (GANs and from-scratch Isolation Forest decision trees) for anomaly and latency behavior analysis.

---

## Architecture Overview

ReconSage is designed as a modular, two-tier system:

- **C++ Native Engine Core (`Native_CPP_build/`)**:
- Direct POSIX socket operations and raw wire-framing without third-party HTTP wrappers.
- Custom byte-level DNS resolution (`ReconDNS`) replacing libc `getaddrinfo()`.
- TLS 1.2+ encryption pipelines layered directly over sockets using OpenSSL.
- Native ML pipelines (`Reco_GAN` and a custom C++ Isolation Forest tree training engine).

- **C# Shell & Orchestration (`ReconSageShell`)**:
- Async CLI shell managing scan lifecycles, configuration sessions, cancellation tokens, and database staging.
- Bridges C# to native shared libraries (`.so`) via high-performance **P/Invoke** interop.

- **`Generic_Module` Subsystem**:
- Transport and socket interface (`wires.cpp`, `SocksModule.cpp`, `interface_scan_module.cpp`) providing unified stream and proxy abstraction across all scan modules.

```
reconsage.1> (C# Shell / SessionData Orchestration)
      │
      ├── Configuration Ingestion (.rfo / .rso / .rxo parsers)
      ├── Generic_Module Bridge (wires.cpp / SocksModule.cpp)
      │
      ├── Scanning Engines (P/Invoke -> C++ .so):
      │     ├── ScanModule.so           → Direct POSIX TCP/TLS probing
      │     ├── TorScan.so              → SOCKS5 + Tor Control Circuit Rotation (SIGNAL NEWNYM)
      │     ├── Proxy_Scanning.so       → HTTP & SOCKS proxy routing
      │     ├── ReconDNS.so             → Wire-level DNS resolver (bypasses libc addrinfo)
      │     └── Response_Body_capture.so→ Raw HTML/payload and header stream extraction
      │
      ├── Storage & Filtering Engine:
      │     └── CompilerToDB.so         → JSON-to-DB indexing, status code & latency filtering
      │
      └── Native ML Engine:
            ├── Reco_GAN                → Generative path prediction & target intelligence
            └── Reco_GAN_Trees (iForest)→ From-scratch C++ Isolation Forest latency analysis

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

Controls scan timings, paths, wordlists, and capture targets.

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

### Configuration Loading

| Command    | Arguments | Description                                                          |
| ---------- | --------- | -------------------------------------------------------------------- |
| `load_rfo` | `<path>`  | Ingests target, DNS, and proxy credentials from a `.rfo` file        |
| `load_rso` | `<path>`  | Ingests scan timings, wordlists, and output paths from a `.rso` file |
| `load_rxo` | `<path>`  | Ingests database and ML parameters from a `.rxo` file                |

### Scanning Modules

| Command                  | Dependencies | Description                                                         |
| ------------------------ | ------------ | ------------------------------------------------------------------- |
| `start_scan_cpp`         | RFO, RSO     | Executes direct POSIX socket scan with OpenSSL TLS and ReconDNS     |
| `start_tor_scan`         | RFO, RSO     | Executes anonymous scan over Tor SOCKS5 with auto-circuit switching |
| `start_http_proxy_scan`  | RFO, RSO     | Routes custom HTTP probing requests through an upstream HTTP proxy  |
| `start_socks_proxy_scan` | RFO, RSO     | Tunnels scan through an arbitrary SOCKS proxy                       |
| `start_cpp_body_capture` | RFO, RSO     | Captures raw response HTML bodies and headers directly to disk      |

### Database & Compiler Pipeline

| Command                                    | Dependencies  | Description                                                              |
| ------------------------------------------ | ------------- | ------------------------------------------------------------------------ |
| `transfer_json_to_db`                      | RFO, RSO, RXO | Indexes generated JSON scan results, headers, and HTML into the database |
| `compile_db_and_based_on_status_code_save` | RFO, RXO      | Filters and compiles database records filtered by HTTP status code       |
| `compile_db_and_based_on_latency_save`     | RFO, RXO      | Filters and compiles database records based on response latency metrics  |

### Native ML & Heuristic Engine

| Command                | Dependencies | Description                                                                                                    |
| ---------------------- | ------------ | -------------------------------------------------------------------------------------------------------------- |
| `reco_gan_training`    | RFO, RXO     | Trains the native GAN model on target behavioral data using `k_factor`                                         |
| `reco_gan_predict`     | RFO, RXO     | Runs generative path and behavior predictions on trained weights                                               |
| `reco_gan_trees_train` | RFO, RXO     | Trains native C++ Isolation Forest trees (`num_trees`, `sub_sample_size`) to detect anomalous latency patterns |
| `exit`                 | None         | Terminates active sessions and shuts down the shell                                                            |

---

## Building the Native C++ Modules

The native core requires **OpenSSL** development headers and a C++17 compliant compiler.

### Build with CMake (Recommended)

```bash
cd Native_CPP_build
mkdir -p build && cd build
cmake ..
make

```

### Build with g++ Directly

```bash
cd Native_CPP_build

# Compile Generic Transport Module
g++ -shared -fPIC -O3 -o Generic_Module.so Generic_Module/*.cpp -lssl -lcrypto

# Compile Core Scan & Capture Engines
g++ -shared -fPIC -O3 -o ScanModule.so ScanModule/*.cpp -lssl -lcrypto
g++ -shared -fPIC -O3 -o TorScan.so TorScan/*.cpp -lssl -lcrypto
g++ -shared -fPIC -O3 -o Proxy_Scanning.so Proxy_Scanning/*.cpp -lssl -lcrypto
g++ -shared -fPIC -O3 -o Response_Body_capture.so Response_Body_capture/*.cpp -lssl -lcrypto

# Compile ML & Tree Engines
g++ -shared -fPIC -O3 -o Reco_GAN.so Reco_GAN/*.cpp
g++ -shared -fPIC -O3 -o CompilerToDB.so Compiler/*.cpp

```

---

## Running the Application

Ensure your compiled `.so` libraries are present in your library lookup path (or working directory), then launch the orchestrator:

```bash
dotnet run

```

---

## System Requirements

- **Operating System:** Linux (POSIX-compliant; optimized for Arch Linux)
- **Runtime:** .NET 8.0+ SDK
- **Compiler:** `cmake` (3.16+) or `g++` (C++17 standard)
- **Libraries:** OpenSSL (`libssl-dev` / `openssl`)
- **Proxy Services (Optional):** Active Tor daemon on port `9050` with Control Port enabled on `9051`

---

## License

This project is licensed under the **GNU General Public License v3.0 (GPL-3.0)**. See the [LICENSE](https://www.google.com/search?q=LICENSE) file in the root repository for the complete license text.

---

## Authorized Testing & Legal Notice

> **Authorized Use Only:** ReconSage is built strictly for authorized network auditing, defensive security engineering, vulnerability assessment, and academic research. Conducting scans or capturing payloads against systems without explicit, documented permission is strictly prohibited by law. The authors and maintainers assume no liability for misuse.
