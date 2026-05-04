#include <cstdint>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <chrono>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <fcntl.h>

using namespace std;

#pragma pack(push, 1)
struct ScanEngineModel
{
    char target[360];
    int32_t status_code;
    char response_headers[65536];
    char reason_phrase[256];
    double latency_ms;
    ScanEngineModel() : status_code(0), latency_ms(0)
    {
        memset(target, 0, 360);
        memset(reason_phrase, 0, 256);
        memset(response_headers, 0, 65536);
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

        // [TADKA] Modern SSL Context
        ctx = SSL_CTX_new(TLS_client_method());

        // Force TLS 1.2 minimum (Google servers strictly require this)
        SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);

        unsigned char alpn[] = {8, 'h', 't', 't', 'p', '/', '1', '.', '1'};
        SSL_CTX_set_alpn_protos(ctx, alpn, sizeof(alpn));
    }

    ~ScanEngine()
    {
        if (ctx)
            SSL_CTX_free(ctx);
    }

    ScanEngineModel *normalScan(const char *target, const char *path, char *port, bool *cancel_flag)
    {
        ScanEngineModel *scan = new ScanEngineModel();
        snprintf(scan->target, sizeof(scan->target), "%s%s", target, path);

        if (cancel_flag && *cancel_flag)
        {
            scan->status_code = -10;
            return scan;
        }

        struct addrinfo hints{}, *res;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(target, port, &hints, &res) != 0)
        {
            scan->status_code = -20;
            return scan;
        }

        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0)
        {
            scan->status_code = -30;
            freeaddrinfo(res);
            return scan;
        }

        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        auto start = chrono::high_resolution_clock::now(); // <-- here

        if (connect(sock, res->ai_addr, res->ai_addrlen) < 0)
        {
            scan->status_code = -1;
            close(sock);
            freeaddrinfo(res);
            return scan;
        }

        bool is_https = (strcmp(port, "443") == 0 || strcmp(port, "https"));    
        SSL *ssl = nullptr;

        if (is_https)
        {
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, sock);
            SSL_set_tlsext_host_name(ssl, target);

            if (SSL_connect(ssl) < 0)
            {
                unsigned long err = ERR_get_error();
                ERR_error_string_n(err, scan->reason_phrase, sizeof(scan->reason_phrase));
                scan->status_code = -50;
                SSL_free(ssl);
                close(sock);
                freeaddrinfo(res);
                return scan;
            }
        }

        char request[1024];
        int req_len = snprintf(request, sizeof(request),
                               "GET %s HTTP/1.1\r\n"
                               "Host: %s\r\n"
                               "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0.0.0 Safari/537.36\r\n"
                               "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*\r\n"
                               "Sec-Ch-Ua: \"Chromium\";v=\"124\", \"Google Chrome\";v=\"124\"\r\n"
                               "Sec-Ch-Ua-Platform: \"Windows\"\r\n"
                               "Connection: close\r\n\r\n",
                               path, target);

        int written = (is_https) ? SSL_write(ssl, request, req_len) : send(sock, request, req_len, 0);
        if (written <= 0)
        {
            scan->status_code = -60;
            if (is_https)
                SSL_free(ssl);
            close(sock);
            freeaddrinfo(res);
            return scan;
        }

        char buff[65536];
        int total_received = 0;

        // Failure Point 6: Reading Response
        while (total_received < (int)sizeof(buff) - 1)
        {
            int bytes_to_read = (int)sizeof(buff) - total_received - 1;
            int bytes = (is_https) ? SSL_read(ssl, buff + total_received, bytes_to_read)
                                   : recv(sock, buff + total_received, bytes_to_read, 0);
            if(bytes == 0)break;
            if (bytes < 0)
            {
                int err = (is_https) ? SSL_get_error(ssl, bytes) : errno;
                scan->status_code = err;
                break;
            }
            total_received += bytes;
            buff[total_received] = '\0';

            char *divider = strstr(buff, "\r\n\r\n");
            if (divider != nullptr)
            {
                *(divider + 2) = '\0';
                total_received = (divider - buff) + 2;
                break; // headers done, stop reading body
            }
        }
        auto end = chrono::high_resolution_clock::now();
        // 1. Better buffer handling after the loop
        if (total_received > 0)
        {
            buff[total_received] = '\0';
            // Safe copy to struct
            strncpy(scan->response_headers, buff, sizeof(scan->response_headers) - 1);

            // 1. Buff mein "HTTP/" dhoondho
            char *http_ptr = strstr(buff, "HTTP/");

            if (http_ptr != nullptr)
            {
                // http_ptr ab "HTTP/1.1 200 OK" point kar raha hai
                // Hum "HTTP/" ke aage 9 characters skip karke status code pe pahunchte hain
                // HTTP/1.1 [SPACE] [3-DIGIT-CODE]
                // 0123456789
                // HTTP/1.1 200

                scan->status_code = atoi(http_ptr + 9);
            }
            else
            {
                // Agar "HTTP/" nahi mila, toh parsing failed
                scan->status_code = -80;
            }

            // Reason phrase mein status line ki pehli line save karo (useful for debug)
            char *line_end = strpbrk(buff, "\r\n");
            if (line_end != nullptr)
            {
                *line_end = '\0'; // Line ko kaat do
                strncpy(scan->reason_phrase, buff, sizeof(scan->reason_phrase) - 1);
            }
        }
        else
        {
            // Agar total_received == 0, toh already -70 set hai (ya error code)
            if (scan->status_code == 0)
            {
                scan->status_code = -70;
            }
        }

        scan->latency_ms = chrono::duration<double, milli>(end - start).count();

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
    ScanEngineModel *PerformScan(void *engine, char *target, char *path, char *port, bool *cancel_flag)
    {
        return static_cast<ScanEngine *>(engine)->normalScan(target, path, port, cancel_flag);
    }
    void DestroyResult(ScanEngineModel *res)
    {
        if (res)
        {
            delete res;
        }
    }
}
