#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include "../Generic_Module/interface_scan_module.cpp"

using namespace std;

class CppScanModule
{
private:
    struct timeval tv;
    SSL_CTX *ctx;
    char domain[256];
    char proto_port[128];
    char headers[8192];
    int timeout;

public:
    CppScanModule(char _domain[256], char _proto_port[128], char _headers[8192], int _timeout)
    {
        timeout = _timeout;
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        
        // FIX 1: Safely copy parameters to internal class fields
        strncpy(domain, _domain, 255);
        domain[255] = '\0';
        strncpy(proto_port, _proto_port, 127);
        proto_port[127] = '\0';
        strncpy(headers, _headers, 8191);
        headers[8191] = '\0';

        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        ctx = SSL_CTX_new(TLS_client_method());
    }

    // FIX 2: Added missing Destructor to clear OpenSSL Context allocation
    ~CppScanModule()
    {
        if (ctx)
        {
            SSL_CTX_free(ctx);
            ctx = nullptr;
        }
    }

    ScanOutputStruct scan(char path[2048], bool *cancel_flag)
    {
        ScanOutputStruct output;
        memset(&output, 0, sizeof(ScanOutputStruct)); // Ensure zero-init state

        GenericInterface interface(domain, headers, proto_port);
        bool isHttps = (strcmp(proto_port, "443") == 0 || strcmp(proto_port, "https") == 0);
        
        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_INET; // Restrict to IPv4 for consistent mapping
        hints.ai_socktype = SOCK_STREAM;
        
        // FIX 3: Fixed error block handling for getaddrinfo
        if (getaddrinfo(domain, proto_port, &hints, &res) != 0)
        {
            return output;
        }
        
        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0)
        {
            freeaddrinfo(res);
            return output;
        }

        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        auto start = chrono::high_resolution_clock::now();
        
        if (connect(sock, res->ai_addr, res->ai_addrlen) < 0)
        {
            cout << "[ERROR C++] Something wrong while trying to connect with the target" << endl;
            close(sock);
            freeaddrinfo(res);
            return output;
        }
        
        freeaddrinfo(res); // Address info can be freed early once connected!
        SSL *ssl = nullptr;

        if (isHttps)
        {
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, sock);
            SSL_set_tlsext_host_name(ssl, domain);

            if (SSL_connect(ssl) <= 0)
            {
                unsigned long err = ERR_get_error();
                ERR_error_string_n(err, output.reason_phrase, sizeof(output.reason_phrase));
                SSL_free(ssl);
                close(sock);
                return output;
            }
        }
        
        // Execution of low-level scan interface
        GenericStruct result = interface.interface_scan(path, cancel_flag, sock, ssl);
        auto end = chrono::high_resolution_clock::now();
        
        // FIX 4: Removed the short-circuit bounds block that dropped successful scans
        char buff[65536];
        memset(buff, 0, sizeof(buff));
        
        strncpy(buff, result.headers, sizeof(buff) - 1);
        strncpy(output.headers, result.headers, sizeof(output.headers) - 1);
        snprintf(output.domain, sizeof(output.domain), "%s", result.domain);
        
        output.status_code = extract_status_from_buffer(buff);
        
        char *line_end = strpbrk(result.headers, "\r\n");
        if (line_end != nullptr)
        {
            *line_end = '\0';
            strncpy(output.reason_phrase, result.headers, sizeof(output.reason_phrase) - 1);
        }
        
        output.latency_ms = chrono::duration<double, milli>(end - start).count();
        return output;
    }
};

extern "C" {
    void *create_engine(char domain[256], char proto_port[128], int timeout, char headers[8192]){
        return new CppScanModule(domain, proto_port, headers, timeout);
    }
    ScanOutputStruct engine_scan(void *engine, char path[2048], bool *cancel_flag){
        if (!engine) return ScanOutputStruct();
        return static_cast<CppScanModule*>(engine)->scan(path, cancel_flag);
    }
    void engine_destroy(void *engine){
        if (engine) {
            delete static_cast<CppScanModule*>(engine);
        }
    }
}