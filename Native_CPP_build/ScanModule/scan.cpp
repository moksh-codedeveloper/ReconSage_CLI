#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
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
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        ctx = SSL_CTX_new(TLS_client_method());
    }

    ScanOutputStruct scan(char path[2048], bool *cancel_flag)
    {
        ScanOutputStruct output;
        GenericInterface interface(domain, headers, proto_port);
        bool isHttps = (strcmp(proto_port, "443") == 0 || strcmp(proto_port, "https") == 0);
        struct addrinfo hints{}, *res;
        hints.ai_protocol = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(domain, proto_port, &hints, &res) < 0)
        {
            freeaddrinfo(res);
            return ScanOutputStruct();
        }
        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        auto start = chrono::high_resolution_clock::now();
        if (connect(sock, res->ai_addr, res->ai_addrlen) < 0)
        {
            cout << "[ERROR C++]Something wrong while trying to connect with the target" << endl;
            close(sock);
            freeaddrinfo(res);
            return ScanOutputStruct();
        }
        SSL *ssl = nullptr;

        if (isHttps)
        {
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, sock);
            SSL_set_tlsext_host_name(ssl, domain);

            if (SSL_connect(ssl) < 0)
            {
                unsigned long err = ERR_get_error();
                ERR_error_string_n(err, output.reason_phrase, sizeof(output.reason_phrase));
                SSL_free(ssl);
                close(sock);
                freeaddrinfo(res);
                return output;
            }
        }
        GenericStruct result = interface.interface_scan(path, cancel_flag, sock, ssl);
        if(sock < 0 || ssl == nullptr){
            freeaddrinfo(res);
            return ScanOutputStruct();
        }
        auto end = chrono::high_resolution_clock::now();
        char buff[65536];
        strncpy(buff, result.headers, 65536);
        strncpy(output.headers, result.headers, 65536);
        output.status_code = extract_status_from_buffer(buff);
        char *line_end = strpbrk(result.headers, "\r\n");
        if(line_end != nullptr){
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
        return static_cast<CppScanModule*>(engine)->scan(path, cancel_flag);
    }

    void engine_destroy(void *engine){
        delete static_cast<CppScanModule*>(engine);
    }
}