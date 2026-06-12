#include "Proxy_Scan.cpp"
#include "Http_Https_generic_modules/interface.cpp"
#include "Http_Https_generic_modules/scan_model.cpp"
#include <chrono>
#include<openssl/ssl.h>
#include<openssl/err.h>
#include"ProxyScanOutputStruct.cpp"

class HttpsScanModule{
    private:
        char domain[256];
        char proto_port[128];
        char headers[8192];
        char proxy_host[256];
        int timeout;
        int proxy_port;
        SSL_CTX *ctx;
        SSL *ssl;
    public:
        HttpsScanModule(char _domain[256], char _proto_port[128], char _headers[8192], char _proxy_host[256], int _proxy_port, int _timeout){
            strncpy(domain, _domain, 256);
            strncpy(proto_port, _proto_port, 128);
            strncpy(proxy_host, _proxy_host, 256);
            strncpy(headers, _headers, 8192);
            proxy_port = _proxy_port;
            timeout = _timeout;
            SSL_library_init();
            OpenSSL_add_ssl_algorithms();
            SSL_load_error_strings();
            ctx = SSL_CTX_new(TLS_client_method());
        }

        ~HttpsScanModule(){
            if(ctx){
                SSL_CTX_free(ctx);
            }
        }

        ProxyScanOutputModel scan(char path[2048]){
            ProxyScanOutputModel result;
            ProxyScan tunnel(domain, proxy_port, proxy_host, proto_port, timeout);
            ScanOutput output;
            UnifiedScanInterface scanInterface(domain, proto_port, headers);
            auto start = chrono::high_resolution_clock::now();
            SSL_Tunnel tunnel_track = tunnel.HttpsTunnel();
            if(tunnel_track.sock < 0){
                return result;
            }
            bool isHttps = (strcmp(proto_port, "443") == 0 || strcmp(proto_port, "https"));
            if(isHttps){
                ssl = SSL_new(ctx);
                BIO *bio = BIO_new(BIO_f_ssl());
                BIO_set_ssl(bio, tunnel_track.proxySsl, BIO_NOCLOSE);
                SSL_set_bio(ssl, bio, bio);
                SSL_set_tlsext_host_name(ssl, domain);
                if(SSL_connect(ssl) < 0){
                    cerr << "Double ssl layer failed at connect part where i connect to target using SSL" << endl;
                    SSL_free(ssl);
                    SSL_free(tunnel_track.proxySsl);
                    close(tunnel_track.sock);
                    return result;
                }
            }
            output = scanInterface.scan(path, tunnel_track.sock, ssl);
            if(tunnel_track.sock < 0){
                if(isHttps){
                    if(ssl == nullptr){
                        return result;
                    }
                }
                return result;
            }
            auto end = chrono::high_resolution_clock::now();
            strncpy(result.headers, output.headers, 65536);
            strncpy(result.domain, output.domain, 3072);
            result.latency_ms = chrono::duration<double, milli>(end - start).count();
            if(isHttps){
                SSL_shutdown(ssl);
                SSL_shutdown(tunnel_track.proxySsl);
                SSL_free(ssl);
                SSL_free(tunnel_track.proxySsl);
            }
            close(tunnel_track.sock);
            return result;
        }
};

extern "C"{
    void *create(char domain[256], char proto_port[128], char proxy_host[256], char headers[8192], int timeout, int proxy_port){
        return new HttpsScanModule(domain, proto_port, headers, proxy_host, proxy_port, timeout);
    }

    ProxyScanOutputModel scan(char path[128], void *engine){
        return static_cast<HttpsScanModule*>(engine)->scan(path);
    }

    void destroy(void *engine){
        delete static_cast<HttpsScanModule*>(engine);
    }
}

