#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include "../ReconDNS/TorDomainStack.cpp"
#include "../ReconDNS/DomainStruct.cpp"
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
using namespace std;

struct BodyDeCaptioStruct
{
    char captured_body[4096];
    char domain[3072];
};

class ReconDeBodyCaptio
{
private:
    char domain[256];
    char dns_server[256];
    char proxy_host[256];
    char proto_port[128];
    int proxy_port;
    int timeout;
    string ip_resolved;
    struct timeval tv;
    SSL_CTX *ctx;

public:
    ReconDeBodyCaptio(char _domain[256], char _dns_server[256], char _proxy_host[256], int _proxy_port, int _timeout, char _proto_port[128])
    {
        strncpy(domain, _domain, 256);
        strncpy(dns_server, _dns_server, 256);
        strncpy(proxy_host, _proxy_host, 256);
        strncpy(proto_port, _proto_port, 128);
        proxy_port = _proxy_port;
        timeout = _timeout;
        TorDnsResolver dns(domain, timeout, dns_server, proxy_host, proxy_port);
        ip_resolved = dns.resolvede();
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        ctx = SSL_CTX_new(TLS_client_method());
    }

    // Recommended add to clean up the context when the class object goes out of scope
    ~ReconDeBodyCaptio()
    {
        if (ctx)
        {
            SSL_CTX_free(ctx);
        }
    }

    BodyDeCaptioStruct scan(bool *cancel_flag, char path[2048])
    {
        BodyDeCaptioStruct myData;
        if (cancel_flag && *cancel_flag)
        {
            return myData;
        }
        snprintf(myData.domain, sizeof(myData.domain), "%s%s", domain, path);
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (cancel_flag && *cancel_flag)
        {
            close(sock);
            return myData;
        }
        struct sockaddr_in target_addr;
        target_addr.sin_family = AF_INET;
        target_addr.sin_addr.s_addr = inet_addr(ip_resolved.c_str());
        if (strcmp(proto_port, "443") == 0)
        {
            target_addr.sin_port = htons(443);
        }
        else if (strcmp(proto_port, "80") == 0)
        {
            target_addr.sin_port = htons(80);
        }
        if (connect(sock, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
        {
            close(sock);
            return BodyDeCaptioStruct();
        }
        if (cancel_flag && *cancel_flag)
        {
            close(sock);
            return myData;
        }
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        bool isHttps = (strcmp(proto_port, "443") == 0);
        SSL *ssl = nullptr;
        if (isHttps)
        {
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, sock);
            SSL_set_tlsext_host_name(ssl, domain);
            if (SSL_connect(ssl) < 0)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
                close(sock);
                return BodyDeCaptioStruct();
            }
        }
        if (cancel_flag && *cancel_flag)
        {
            if (isHttps)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            close(sock);
            return myData;
        }
        char request[512];
        int req_len = snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, domain);
        int bytes_read = (isHttps) ? SSL_write(ssl, request, req_len) : send(sock, request, req_len, 0);
        char buff[8192];
        size_t totalBytesRecieved = 0;
        char *body_start = nullptr;
        if (cancel_flag && *cancel_flag)
        {
            if (isHttps)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            close(sock);
            return myData;
        }
        while (true)
        {
            size_t remainingSpace = sizeof(buff) - totalBytesRecieved - 1;
            if (remainingSpace <= 0)
                break;
            ssize_t bytesRead = (isHttps) ? SSL_read(ssl, buff + totalBytesRecieved, remainingSpace) : recv(sock, buff + totalBytesRecieved, remainingSpace, 0);
            if (bytesRead <= 0)
                break;
            totalBytesRecieved += bytesRead;
            buff[totalBytesRecieved] = '\0'; // Always keep it null-terminated
            if (cancel_flag && *cancel_flag)
            {
                if (isHttps)
                {
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
                }
                close(sock);
                return myData;
            }
            // Scan the accumulated buff to see if the headers have cleared yet
            if (body_start == nullptr)
            {
                char *headerEnd = std::strstr(buff, "\r\n\r\n");
                if (headerEnd != nullptr)
                {
                    body_start = headerEnd + 4; // Map the exact pointer location of the body
                }
            }
            if (cancel_flag && *cancel_flag)
            {
                if (isHttps)
                {
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
                }
                close(sock);
                return myData;
            }

            // Optimization: Once we have the body pointer, check if we have harvested
            // enough body bytes (e.g., 4096 bytes) to capture the error zone.
            if (body_start != nullptr)
            {
                size_t currentBodySize = (buff + totalBytesRecieved) - body_start;
                if (currentBodySize >= 4096)
                {
                    break; // We have exactly what we need, break early!
                }
            }
            if (cancel_flag && *cancel_flag)
            {
                if (isHttps)
                {
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
                }
                close(sock);
                return myData;
            }
        }
        if (cancel_flag && *cancel_flag)
        {
            if (isHttps)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            close(sock);
            return myData;
        }
        // --- THE ONLY CHANGE MADE BELOW ---
        if (isHttps && ssl)
        {
            SSL_shutdown(ssl);
            SSL_free(ssl); // Frees the SSL pointer structure on a successful connection exit
        }
        close(sock);
        // ----------------------------------

        if (body_start != nullptr)
        {
            size_t availableBodyBytes = (buff + totalBytesRecieved) - body_start;
            size_t bytesToCopy = (availableBodyBytes > 4095) ? 4095 : availableBodyBytes;
            std::memcpy(myData.captured_body, body_start, bytesToCopy);
            myData.captured_body[bytesToCopy] = '\0';
        }
        return myData;
    }
};

extern "C"
{
    void *create_res_body_capture_engine(char domain[256], char proxy_host[256], char proto_port[128], char dns_server[256], int proxy_port, int timeout)
    {
        return new ReconDeBodyCaptio(domain, dns_server, proxy_host, proxy_port, timeout, proto_port);
    }
    BodyDeCaptioStruct res_cap_scan(void *engine, char path[2048], bool *cancel_flag)
    {
        return static_cast<ReconDeBodyCaptio *>(engine)->scan(cancel_flag, path);
    }
}