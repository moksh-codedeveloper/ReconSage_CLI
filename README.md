# ReconSage CLI

**ReconSage** is a behavioral reconnaissance scanner built from the ground up in **C++ and C#**.

It doesn't just discover targets — it observes *how* a server behaves under different conditions, captures response headers, measures latency, and outputs structured JSON reports.

---

## Architecture

ReconSage is a two-language system:

- **C++ scan engines** — built on the **POSIX standard** using raw OS syscalls (`socket()`, `connect()`, `send()`, `recv()`). No HTTP libraries. No abstractions. Direct metal.
- **C# shell + orchestration** — interactive CLI shell that loads config, drives the C++ engines via **P/Invoke**, and handles output.
- **Shared `.so` bridge** — C++ modules compile to shared libraries with `extern "C"` linkage, loaded at runtime by the C# layer.

```
reconsage.1> (C# shell)
      │
      ├── load_rso / load_rfo   → parse config files
      ├── start_scan_cpp        → P/Invoke → ScanEngine.so (POSIX sockets + OpenSSL)
      └── start_tor_scan        → P/Invoke → TorScan.so   (SOCKS5 + Tor control port)
```

---

## Scan Modules

### Normal Scan (`start_scan_cpp`)
- Raw TCP sockets via POSIX `socket()` / `connect()`
- DNS resolution via `getaddrinfo()`
- HTTP and HTTPS support (TLS 1.2+ enforced via OpenSSL)
- Captures: status code, response headers, reason phrase, latency (ms)
- Cancel flag support for graceful interruption

### Tor Scan (`start_tor_scan`)
- Manual **SOCKS5 handshake** implemented byte-by-byte
- Tunnels all traffic through Tor (`tor_ip:tor_port`)
- Auto **circuit rotation** via Tor control port (`SIGNAL NEWNYM`)
- TLS support on top of the SOCKS5 tunnel
- Configurable control port password

---

## Config Files

ReconSage is config-file driven. Two files are required before scanning.

### RFO — Recon File Options (target config)

```ini
[target]
target      = example.com
proto_port  = 443
tor_ip      = 127.0.0.1
tor_port    = 9050
cp_port     = 9051
password    = your_tor_password
```

| Field | Description |
|---|---|
| `target` | Target hostname (no `http://`) |
| `proto_port` | Port to scan (`80` or `443`) |
| `tor_ip` | Tor SOCKS5 proxy IP (usually `127.0.0.1`) |
| `tor_port` | Tor SOCKS5 port (default `9050`) |
| `cp_port` | Tor control port (default `9051`) |
| `password` | Tor control port authentication password |

### RSO — Recon Scan Options (scan behavior)

```ini
[scan]
timeout       = 5000
delay         = 100
wordlist_path = /path/to/wordlist.txt
json_file_path = /path/to/output.json
```

| Field | Description |
|---|---|
| `timeout` | Request timeout in milliseconds |
| `delay` | Delay between requests in milliseconds |
| `wordlist_path` | Path to `.txt` wordlist file |
| `json_file_path` | Output path for JSON report (must end in `.json`) |

---

## Usage

### 1. Launch ReconSage

```bash
dotnet run
```

You'll see the interactive shell:

```
reconsage.1>
```

### 2. Load your config files

```
reconsage.1> load_rfo /path/to/target.rfo
reconsage.1> load_rso /path/to/scan.rso
```

### 3. Run a scan

**Normal scan (direct POSIX sockets):**
```
reconsage.1> start_scan_cpp
```

**Tor scan (anonymous via SOCKS5):**
```
reconsage.1> start_tor_scan
```

### 4. Output

During the scan, live progress is printed to the terminal. When the scan completes, full results are saved to the JSON path you specified in your `.rso` file.

```json
{
  "Result": [
    {
      "Target": "example.com/admin",
      "StatusCode": 403,
      "ReasonPhrase": "HTTP/1.1 403 Forbidden",
      "LatencyMs": 212.4,
      "ResponseHeaders": "..."
    }
  ]
}
```

---

## Requirements

- **Linux** (POSIX — developed on Arch Linux)
- **.NET SDK** (for the C# shell)
- **g++** with OpenSSL (`libssl-dev`)
- **Tor** running locally (for `start_tor_scan`)

---

## Building the C++ Modules

Each scan engine compiles to a shared library:

```bash
g++ -shared -fPIC -o ScanEngine.so ScanEngine.cpp -lssl -lcrypto
g++ -shared -fPIC -o TorScan.so TorScan.cpp -lssl -lcrypto
```

---

## Commands

| Command | Description |
|---|---|
| `load_rso <path>` | Load scan options from `.rso` file |
| `load_rfo <path>` | Load target config from `.rfo` file |
| `start_scan_cpp` | Run normal scan via C++ POSIX engine |
| `start_tor_scan` | Run anonymous scan via Tor + SOCKS5 |
| `exit` | Exit ReconSage |

---

## Ethical Notice

ReconSage is built for **security research and authorized testing only**.

- Only scan targets you own or have explicit permission to test.
- The Tor scan module sends real traffic through the Tor network — use responsibly.
- Misuse of this tool may violate laws and program rules.

---

## About

ReconSage is built out of curiosity and persistence.

> *Never give up because of errors. If you want to build, you will figure it out.*

**Stack:** C++ · C# · POSIX Sockets · OpenSSL · SOCKS5 · Tor · P/Invoke · Arch Linux