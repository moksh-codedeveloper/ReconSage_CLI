#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <openssl/ssl.h>
#include <openssl/err.h>
using namespace std;

struct SSL_Tunnel
{
    int sock = -1;
    SSL *proxySsl = nullptr;
};

class ProxyScan
{
private:
    char domain[256];
    char proxy_host[256];
    int proxy_port;
    char port[128];
    SSL *proxy_ssl;
    SSL_CTX *ctx;
    struct timeval tv;
public:
    ProxyScan(char target[256], int _proxy_port, char host[256], char _port[128], int timeout)
    {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        strncpy(domain, target, 255);
        strncpy(proxy_host, host, 255);
        proxy_port = _proxy_port;
        strncpy(port, _port, 127);
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        ctx = SSL_CTX_new(TLS_client_method());
    }
    ~ProxyScan()
    {
        if (ctx)
            SSL_CTX_free(ctx);
    }
    int HttpProxy()
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(proxy_port);
        inet_pton(AF_INET, proxy_host, &addr.sin_addr);
        if (connect(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
        {
            cerr << "[proxy_http_error] you are encountering a error with the proxy you provided" << endl;
            close(sock);
            return -1;
        }
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        char req[512];
        snprintf(req, sizeof(req),
                 "CONNECT %s:%s HTTP/1.1\r\nHost: %s:%s\r\n\r\n",
                 domain, port, domain, port);
        send(sock, req, strlen(req), 0);

        char response_headers[512] = {};
        recv(sock, response_headers, sizeof(response_headers), 0);

        if (strstr(response_headers, "200") == nullptr)
        {
            cerr << "i think proxy is either denying or something else is going on with your proxy..." << endl;
            close(sock);
            return -1;
        }
        cout << "tunnnel is working and is connected ...." << endl;
        return sock;
    }

    // SSL_Tunnel HttpsTunnel()
    // {
    //     SSL_Tunnel sslTunnel; 
    //     int sock = socket(AF_INET, SOCK_STREAM, 0);
    //     if (sock < 0)
    //         return sslTunnel;

    //     sockaddr_in addr{};
    //     addr.sin_family = AF_INET;
    //     addr.sin_port = htons(proxy_port);
    //     inet_pton(AF_INET, proxy_host, &addr.sin_addr);

    //     if (connect(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
    //     {
    //         cerr << "CONNECTION TO PROXY FAILED....." << endl;
    //         close(sock);
    //         return sslTunnel;
    //     }
    //     setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    //     proxy_ssl = SSL_new(ctx);
    //     SSL_set_fd(proxy_ssl, sock);
    //     SSL_set_tlsext_host_name(proxy_ssl, proxy_host);

    //     if (SSL_connect(proxy_ssl) <= 0)
    //     {
    //         char err_buff[1024];
    //         ERR_error_string_n(ERR_get_error(), err_buff, sizeof(err_buff));
    //         cerr << "Proxy SSL Handshake failed: " << err_buff << endl;
    //         SSL_free(proxy_ssl);
    //         close(sock);
    //         return sslTunnel;
    //     }

    //     char req[512];
    //     snprintf(req, sizeof(req),
    //              "CONNECT %s:%s HTTP/1.1\r\n"
    //              "Host: %s:%s\r\n\r\n",
    //              domain, port, domain, port);

    //     SSL_write(proxy_ssl, req, (int)strlen(req));

    //     char response[512] = {0};
    //     int r = SSL_read(proxy_ssl, response, sizeof(response) - 1);

    //     if (r <= 0 || strstr(response, "200") == nullptr)
    //     {
    //         cerr << "Proxy denied CONNECT. Response: " << response << endl;
    //         SSL_free(proxy_ssl);
    //         close(sock);
    //         return sslTunnel;
    //     }

    //     cout << "tunnnel is working and is connected ...." << endl;

    //     sslTunnel.sock = sock;
    //     sslTunnel.proxySsl = proxy_ssl;
    //     return sslTunnel;
    // }
};