#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>

// Mocking these imports so it compiles out-of-the-box, keep yours!
class TorDnsResolver {
public:
    TorDnsResolver(char* d, int t, char* s, char* ph, int pp) {}
    std::string resolvede() { return "142.250.190.36"; } // Google IPv4 Mock
};

using namespace std;

struct BodyDeCaptioStruct
{
    char captured_body[4096];
    char domain[256];
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

    // FIX #2: Added Destructor to free the SSL Context context structure out of kernel memory
    ~ReconDeBodyCaptio() {
        if (ctx) {
            SSL_CTX_free(ctx);
        }
    }

    BodyDeCaptioStruct scan(bool *cancel_flag)
    {
        BodyDeCaptioStruct myData;
        // FIX #3: Guarantee the struct arrays start clean and null-terminated
        std::memset(&myData, 0, sizeof(myData)); 

        char path[] = "/this-path-does-not-exist";
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        
        struct sockaddr_in target_addr;
        target_addr.sin_family = AF_INET;
        target_addr.sin_addr.s_addr = inet_addr(ip_resolved.c_str());
        
        if (strcmp(proto_port, "443") == 0) {
            target_addr.sin_port = htons(443);
        } else {
            target_addr.sin_port = htons(80);
        }

        if (connect(sock, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
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
                SSL_free(ssl); // Don't call shutdown if connect failed, just free resource
                close(sock);
                return myData;
            }
        }

        char request[512];
        int req_len = snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, domain);
        
        // Write out payload
        if (isHttps) {
            SSL_write(ssl, request, req_len);
        } else {
            send(sock, request, req_len, 0);
        }

        char buff[8192];
        size_t totalBytesRecieved = 0;
        char *body_start = nullptr;

        while (true)
        {
            // Cancel flag processing optimization check
            if (cancel_flag && *cancel_flag) break;

            size_t remainingSpace = sizeof(buff) - totalBytesRecieved - 1;
            if (remainingSpace <= 0)
                break;

            ssize_t bytesRead = (isHttps) ? SSL_read(ssl, buff + totalBytesRecieved, remainingSpace) : recv(sock, buff + totalBytesRecieved, remainingSpace, 0);
            if (bytesRead <= 0)
                break;

            totalBytesRecieved += bytesRead;
            buff[totalBytesRecieved] = '\0'; 

            if (body_start == nullptr)
            {
                char *headerEnd = std::strstr(buff, "\r\n\r\n");
                if (headerEnd != nullptr)
                {
                    body_start = headerEnd + 4; 
                }
            }

            if (body_start != nullptr)
            {
                size_t currentBodySize = (buff + totalBytesRecieved) - body_start;
                if (currentBodySize >= 4096)
                {
                    break; 
                }
            }
        }

        // FIX #1: Clean up SSL allocations on a successful run before ripping down the raw file descriptor
        if (isHttps && ssl) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        close(sock);

        // Copy stage
        if (body_start != nullptr)
        {
            size_t availableBodyBytes = (buff + totalBytesRecieved) - body_start;
            size_t bytesToCopy = (availableBodyBytes > 4095) ? 4095 : availableBodyBytes;
            std::memcpy(myData.captured_body, body_start, bytesToCopy);
            myData.captured_body[bytesToCopy] = '\0';
        }
        
        strncpy(myData.domain, domain, sizeof(myData.domain) - 1);
        return myData;
    }
};

int main(){
    char domain[] = "www.google.com";
    char proxy_host[] = "127.0.0.1";
    char dns_server[] = "1.1.1.1";
    char proto_port[] = "443";
    int proxy_port = 9050;
    int timeout = 5000;
    bool cancel_flag = false;

    ReconDeBodyCaptio recon(domain, dns_server, proxy_host, proxy_port, timeout, proto_port);
    BodyDeCaptioStruct reconResult = recon.scan(&cancel_flag);

    cout << "Success" << endl;
    cout << "-------------------------" << endl;
    cout << "Domain :- " << reconResult.domain << endl;
    cout << "Body :- \n" << reconResult.captured_body << endl;
    cout << "-------------------------" << endl;
    return 0;
}