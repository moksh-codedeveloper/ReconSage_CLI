#include <cstdint>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <chrono>
#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace std;

#pragma pack(push, 1)
struct ScanEngineModel
{
    char target[360];
    int32_t status_code;
    char *response_headers;
    char reason_phrase[256];
    double latency_ms;
    ScanEngineModel() : status_code(0), latency_ms(0), response_headers(nullptr)
    {
        memset(target, 0, 360);
        memset(reason_phrase, 0, 256);
    }
};

class ScanEngine
{
private:
    struct timeval tv;
    SSL_CTX *ctx;

public:
    ScanEngine(int timeout)
    {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        // OpenSSL Initialization
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        ctx = SSL_CTX_new(TLS_client_method());
    }

    ~ScanEngine()
    {
        if (ctx)
            SSL_CTX_free(ctx);
    }

    ScanEngineModel *normalScan(char target[256], char path[100], char port[128], bool *cancel_flag)
    {
        if (cancel_flag && *cancel_flag)
            return nullptr;
        ScanEngineModel *scan = new ScanEngineModel();

        struct addrinfo hints{}, *res;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(target, port, &hints, &res) != 0)
        {
            return scan;
        }
        snprintf(scan->target, sizeof(scan->target), "%s%s", target, path);
        if (cancel_flag && *cancel_flag)
        {
            if (res)
                freeaddrinfo(res);
            return scan;
        }

        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        auto start = chrono::high_resolution_clock::now();

        if (connect(sock, res->ai_addr, res->ai_addrlen) < 0)
        {
            scan->status_code = -1;
            close(sock);
            freeaddrinfo(res);
            return scan;
        }
        // Common Cleanup Macro/Function (Just for your mental map)
        if (cancel_flag && *cancel_flag)
        {
            if (sock >= 0)
                close(sock); // Close only if valid
            if (res)
                freeaddrinfo(res);
            return scan;
        }
        bool is_https = (strcmp(port, "443") == 0);
        SSL *ssl = nullptr;

        if (is_https)
        {
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, sock);
            // SNI (Server Name Indication) - Google ke liye zaroori hai
            SSL_set_tlsext_host_name(ssl, target);

            if (SSL_connect(ssl) <= 0)
            {
                scan->status_code = -2; // TLS Handshake Failed
                SSL_free(ssl);
                close(sock);
                freeaddrinfo(res);
                return scan;
            }
        }

        // Common Cleanup Macro/Function (Just for your mental map)
        if (cancel_flag && *cancel_flag)
        {
            if (ssl)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            } // Check before killing SSL
            if (sock >= 0)
                close(sock); // Close only if valid
            if (res)
                freeaddrinfo(res);
            return scan;
        }

        char request[512];
        int req_len = snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, target);

        if (is_https)
            SSL_write(ssl, request, req_len);
        else
            send(sock, request, req_len, 0);

        size_t capacity = 65536;
        char *buff = (char *)malloc(capacity);
        int total_received = 0;
        int bytes;
        // Common Cleanup Macro/Function (Just for your mental map)
        if (cancel_flag && *cancel_flag)
        {
            if (ssl)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            } // Check before killing SSL
            if (sock >= 0)
                close(sock); // Close only if valid
            if (res)
                freeaddrinfo(res);
            if (buff)
                free(buff); // Avoid double free
            return scan;
        }
        // Reading logic (SSL vs Plain)
        while (true)
        {
            if (is_https)
                bytes = SSL_read(ssl, buff + total_received, capacity - total_received - 1);
            else
                bytes = recv(sock, buff + total_received, capacity - total_received - 1, 0);

            if (bytes <= 0)
                break;

            total_received += bytes;
            buff[total_received] = '\0';
            // Common Cleanup Macro/Function (Just for your mental map)
            if (cancel_flag && *cancel_flag)
            {
                if (ssl)
                {
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
                } // Check before killing SSL
                if (sock >= 0)
                    close(sock); // Close only if valid
                if (res)
                    freeaddrinfo(res);
                if (buff)
                    free(buff); // Avoid double free
                return scan;
            }
            char *divider = strstr(buff, "\r\n\r\n");
            if (divider)
            {
                size_t header_len = divider - buff;
                scan->response_headers = (char *)malloc(header_len + 1);
                if (cancel_flag && *cancel_flag)
                {
                    if (ssl)
                    {
                        SSL_shutdown(ssl);
                        SSL_free(ssl);
                    } // Check before killing SSL
                    if (sock >= 0)
                    {
                        close(sock);
                    } // Close only if valid
                    if (res)
                    {
                        freeaddrinfo(res);
                    }
                    if (buff)
                    {
                        free(buff); // Avoid double free
                    }
                    if (scan->response_headers)
                    {
                        free(scan->response_headers);
                        scan->response_headers = nullptr;
                    }
                    return scan;
                }
                memcpy(scan->response_headers, buff, header_len);
                scan->response_headers[header_len] = '\0';
                sscanf(scan->response_headers, "HTTP/%*f %d %255[^\r\n]", &scan->status_code, scan->reason_phrase);
                break;
            }
        }

        auto end = chrono::high_resolution_clock::now();
        scan->latency_ms = chrono::duration<double, milli>(end - start).count();
        // Common Cleanup Macro/Function (Just for your mental map)
        if (cancel_flag && *cancel_flag)
        {
            if (ssl)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            } // Check before killing SSL
            if (sock >= 0)
                close(sock); // Close only if valid
            if (res)
                freeaddrinfo(res);
            if (buff)
                free(buff); // Avoid double free
            return scan;
        }
        free(buff);
        if (is_https)
        {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        close(sock);
        freeaddrinfo(res);
        return scan;
    }
};

extern "C"
{
    void *CreateEngine(int timeoutms) { return new ScanEngine(timeoutms); }
    ScanEngineModel *PerformScan(void *engine, char target[256], char path[100], char port[128], bool *cancel_flag)
    {
        return static_cast<ScanEngine *>(engine)->normalScan(target, path, port, cancel_flag);
    }
    void DestroyResult(ScanEngineModel *res)
    {
        if (res)
        {
            if (res->response_headers)
                free(res->response_headers);
            delete res;
        }
    }
}
