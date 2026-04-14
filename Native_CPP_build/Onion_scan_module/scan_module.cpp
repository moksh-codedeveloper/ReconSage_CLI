#include <cstdint>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <chrono>

using namespace std;

// The data packet for the C# Brain
struct ScanModule {
    char target[357];
    int status_code;
    char* response_headers;
    double latency_ms;
    char reason_phrase[100];

    ScanModule() : response_headers(nullptr), status_code(0), latency_ms(0) {
        memset(target, 0, 357);
        memset(reason_phrase, 0, 100);
    }

    ~ScanModule() {
        if (response_headers) delete[] response_headers;
        // reason_phrase is a fixed array, no need to delete it!
    }
};

class ReconEngine {
private:
    struct timeval tv; // Linux uses timeval for socket timeouts

public:
    ReconEngine(int timeout) {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
    }

    ScanModule* scan(const char* target_host, const char* path) {
        ScanModule* result = new ScanModule();
        struct addrinfo hints{}, *res;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(target_host, "80", &hints, &res) != 0) return result;

        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        
        // Apply the timeout
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
            close(sock);
            freeaddrinfo(res);
            return result;
        }

        // Craft the Surgical Strike Request
        char request[512];
        snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, target_host);

        auto start = chrono::high_resolution_clock::now();
        send(sock, request, strlen(request), 0);

        char buff[4096] = {0};
        int bytes = recv(sock, buff, sizeof(buff) - 1, 0);
        auto end = chrono::high_resolution_clock::now();

        result->latency_ms = chrono::duration<double, milli>(end - start).count();
        strncpy(result->target, target_host, 356);

        if (bytes > 0) {
            result->response_headers = new char[bytes + 1];
            memcpy(result->response_headers, buff, bytes);
            result->response_headers[bytes] = '\0';

            // Extract the truth from the raw buffer
            sscanf(buff, "HTTP/%*f %d %99[^\r\n]", &result->status_code, result->reason_phrase);
        }

        close(sock);
        freeaddrinfo(res);
        return result;
    }
};

// --- THE BRIDGE ---
extern "C" {
    void* CreateEngine(int timeout) { return new ReconEngine(timeout); }
    
    ScanModule* PerformScan(void* engine, const char* host, const char* path) {
        return static_cast<ReconEngine*>(engine)->scan(host, path);
    }

    void DestroyResult(ScanModule* res) { delete res; }
}