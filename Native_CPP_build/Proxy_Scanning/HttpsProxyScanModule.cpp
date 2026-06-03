#include "Proxy_Scan.cpp"
#include "Http_Https_generic_modules/interface.cpp"
#include <chrono>

struct HttpsScanOutput
{
    char full_target_path[2048];
    char reason_phrase[128];
    char response_headers[65536];
    int status_code;
    double latency_ms;
};

class HttpsProxyModule
{
private:
    char domain[256];
    char proxy_host[256];
    char port[128];
    int proxy_port;
    int timeout;
    SSL_CTX *ctx;
    char headers[5120];

public:
    HttpsProxyModule(char target[256], char _proxy_host[256], char _port[128], int _proxy_port, int _timeout, char header[6112])
    {
        strncpy(domain, target, 255);
        strncpy(proxy_host, _proxy_host, 255);
        strncpy(port, _port, 127);
        strncpy(headers, header, 6112);
        proxy_port = _proxy_port;
        timeout = _timeout;
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        ctx = SSL_CTX_new(TLS_client_method());
        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    }
    ~HttpsProxyModule()
    {
        if (ctx)
            SSL_CTX_free(ctx);
    }
    int extract_status_from_buffer(const char *buffer)
    {
        if (!buffer)
            return -1;

        const char *ptr = buffer;
        while (*ptr != '\0' && *ptr != ' ' && *ptr != '\r' && *ptr != '\n')
        {
            ptr++;
        }
        if (*ptr != ' ')
        {
            return -1;
        }
        ptr++;
        int code = 0;
        for (int i = 0; i < 3; ++i)
        {
            if (*ptr >= '0' && *ptr <= '9')
            {
                code = code * 10 + (*ptr - '0');
                ptr++;
            }
            else
            {
                return -1;
            }
        }

        return code;
    }
    HttpsScanOutput HttpsMainScan(char path[2048])
    {
        ProxyScan proxyScan(domain, proxy_port, proxy_host, port, timeout);

        HttpsScanOutput proxyScanOutput;
        snprintf(proxyScanOutput.full_target_path, sizeof(proxyScanOutput.full_target_path), "%s%s", domain, path);
        auto start = chrono::high_resolution_clock::now();
        SSL_Tunnel sessionData = proxyScan.HttpsTunnel();
        if (sessionData.sock == -1)
            return proxyScanOutput;
        SSL *targetSSL = SSL_new(ctx);
        BIO *bio = BIO_new(BIO_f_ssl());
        BIO_set_ssl(bio, sessionData.proxySsl, BIO_NOCLOSE);
        SSL_set_bio(targetSSL, bio, bio);
        SSL_set_tlsext_host_name(targetSSL, domain);
        if (SSL_connect(targetSSL) <= 0)
        {
            cerr << "Double SSL Handshake Failed!" << endl;
            SSL_free(targetSSL);
            SSL_free(sessionData.proxySsl);
            close(sessionData.sock);
            return proxyScanOutput;
        }
        char req[7168];
        int req_len = snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s\r\n%s", path, domain, headers);
        int bytes_written = SSL_write(targetSSL, req, req_len);
        if (bytes_written <= 0)
        {
            proxyScanOutput.status_code = -60;
            SSL_free(targetSSL);
            SSL_free(sessionData.proxySsl);
            close(sessionData.sock);
            return proxyScanOutput;
        }
        else
        {
            char buff[65536];
            int total_received = 0;
            // int bytes_read = SSL_read(sessionData.proxySsl, buff, sizeof(buff) - 1);
            while (total_received < (int)sizeof(buff) - 1)
            {
                int bytes_to_read = (int)sizeof(buff) - total_received - 1;
                int bytes = SSL_read(targetSSL, buff + total_received, bytes_to_read);
                if (bytes == 0)
                    break;
                if (bytes < 0)
                {
                    int err = SSL_get_error(targetSSL, bytes);
                    proxyScanOutput.status_code = err;
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

            if (total_received > 0)
            {
                buff[total_received] = '\0';
                strncpy(proxyScanOutput.response_headers, buff, sizeof(proxyScanOutput.response_headers) - 1);
                proxyScanOutput.status_code = extract_status_from_buffer(buff);
                char *line_end = strpbrk(buff, "\r\n");
                if (line_end != nullptr)
                {
                    *line_end = '\0';
                    strncpy(proxyScanOutput.reason_phrase, buff, sizeof(proxyScanOutput.reason_phrase) - 1);
                }
            }
            proxyScanOutput.latency_ms = chrono::duration<double, milli>(end - start).count();
        }
        SSL_free(targetSSL);
        close(sessionData.sock);
        SSL_free(sessionData.proxySsl);
        return proxyScanOutput;
    }
};

extern "C" {
    void *create(char *domain, char *proto_port, char *proxy_host, char *headers, int proxy_port, int timeout){
        return new HttpsProxyModule(domain, proxy_host, proto_port, proxy_port, timeout, headers);
    }

    HttpsScanOutput scan(char *path, void *engine){
        return static_cast<HttpsProxyModule*>(engine)->HttpsMainScan(path);
    }

    void destroy(void *engine){
        delete static_cast<HttpsProxyModule*>(engine);
    }
}


