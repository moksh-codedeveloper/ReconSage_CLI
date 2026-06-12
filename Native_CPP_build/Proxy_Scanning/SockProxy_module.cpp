#include"Proxy_Scan.cpp"
#include<chrono>
#include"Http_Https_generic_modules/interface.cpp"
#include"Http_Https_generic_modules/scan_model.cpp"
#include"ProxyScanOutputStruct.cpp"
#include<openssl/ssl.h>
#include<openssl/err.h>
using namespace std;

class SocksScan{
private:
    char domain[256];
    char proto_port[128];
    int proxy_port;
    char proxy_host[256];
    char headers[8192];
    int timeout;
    SSL_CTX *ctx;
    SSL *ssl;
public:
    SocksScan(char _domain[256], char _proto_port[128], char _proxy_host[256], char _headers[8192], int _proxy_port, int _timeout){

        strncpy(domain, _domain, 256);
        strncpy(proto_port, _proto_port, 128);
        strncpy(proxy_host, _proxy_host, 256);
        proxy_port = _proxy_port;
        timeout = _timeout;
        strncpy(headers, _headers, 8192);
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        ctx = SSL_CTX_new(TLS_client_method());

    }

    ~SocksScan(){
        if(ctx)
            SSL_CTX_free(ctx);
    }

    ProxyScanOutputModel scan(char path[2048]){
        ProxyScanOutputModel result;
        ScanOutput output;
        ProxyScan proxyScan(domain, proxy_port, proxy_host, proto_port, timeout);
        UnifiedScanInterface interface(domain, proto_port, headers);
        bool isHttps = (strcmp(proto_port, "https") == 0 || strcmp(proto_port, "443") == 0);

        auto start = chrono::high_resolution_clock::now();
        int sock = proxyScan.SocksTunnel();
        if(isHttps){
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, sock);
            SSL_set_tlsext_host_name(ssl, domain);
            if(SSL_connect(ssl) < 0){
                SSL_free(ssl);
                close(sock);
                return result;
            }
        }

        output = interface.scan(path, sock, ssl);

        if(sock < 0){
            if(isHttps){
                if(ssl == nullptr){
                    return result;
                }
            }
            return result;
        }
        auto end = chrono::high_resolution_clock::now();

        strncpy(result.domain, output.domain, 3072);
        strncpy(result.headers, output.headers, 65536);
        result.status_code = extract_status_from_buffer(output.headers);
        result.latency_ms = chrono::duration<double, milli>(end - start).count();

        char *line_end = strpbrk(output.headers, "\r\n");

        if(line_end != nullptr){
            *line_end = '\0';
            strncpy(result.reason_phrase, output.headers, sizeof(result.reason_phrase) - 1);
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(sock);
        return result;
    }
};

extern "C" {
    void *create(char *domain, char *proxy_host, char *proto_port, char *headers, int timeout, int proxy_port){
        return new SocksScan(domain, proto_port, proxy_host, headers, proxy_port, timeout);
    }

    ProxyScanOutputModel scan(char *path, void *engine){
        return static_cast<SocksScan*>(engine)->scan(path);
    }

    void destroy(void *engine){
        delete static_cast<SocksScan*>(engine);
    }
}

