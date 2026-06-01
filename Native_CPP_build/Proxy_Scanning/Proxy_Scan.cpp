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
    int SocksTunnel()
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(proxy_port);
        inet_pton(AF_INET, proxy_host, &serv_addr.sin_addr);
        size_t target_len = strlen(domain);
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("Connection Error");
        }
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        char handshake[] = {0x05, 0x01, 0x00};
        char response[2];
        send(sock, handshake, 3, 0);
        memset(response, 0, 2);
        recv(sock, response, 2, 0);
        if (response[0] == 0x05 && response[1] == 0x00)
        {
            vector<uint8_t> greeting;
            greeting.push_back(0x05); // socks version :- 5
            greeting.push_back(0x01); // cmd connect
            greeting.push_back(0x00); // rsv
            greeting.push_back(0x03);
            greeting.push_back((uint8_t)target_len);
            for (int i = 0; i < target_len; i++)
            {
                greeting.push_back(domain[i]);
            }
            uint16_t port_val = htons(atoi(port));
            uint8_t port_bytes[2];
            memcpy(port_bytes, &port_val, 2);

            greeting.push_back(port_bytes[0]);
            greeting.push_back(port_bytes[1]);
            uint8_t res[2];
            memset(res, 0, 2);
            send(sock, greeting.data(), greeting.size(), 0);
            int total_received = 0;
            while (total_received < 2)
            {
                int bytes = recv(sock, res + total_received, 2 - total_received, 0);
                if (bytes <= 0)
                    break;
                total_received += bytes;
            }
            if (res[0] == 0x05 && res[1] == 0x00)
            {
                return sock;
            }
            else if (res[0] == 0x05 && res[1] == 0x01)
            {
                cout << "general SOCKS server failure" << endl;
                cout << (int)res[0] << " " << (int)res[1] << endl;
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x02)
            {
                cout << "connection not allowed by rulese" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x03)
            {
                cout << "Network unreachable" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x04)
            {
                cout << "Host unreachable " << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x05)
            {
                cout << "Connection refused" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x06)
            {
                cout << "TTL expired" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x07)
            {
                cout << "Command not supported" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x08)
            {
                cout << "Address type not supported" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x09)
            {
                cout << "to X’FF’ unassigned" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                return -1;
            }
            else
            {
                cout << (int)res[0] << (int)res[1] << endl;
                return -1;
            }
        }
        else
        {
            cout << "[ERROR]" << "Your connection got refused or somethinng error happened" << (int)response[1] << endl;
            return -1;
        }
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

    SSL_Tunnel HttpsTunnel()
    {
        SSL_Tunnel sslTunnel; // Initialized as {-1, nullptr}

        // 1. Raw Socket create karo
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
            return sslTunnel;

        // 2. Proxy address setup (127.0.0.1:8080)
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(proxy_port);
        inet_pton(AF_INET, proxy_host, &addr.sin_addr);

        // 3. Connect to Proxy
        if (connect(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
        {
            cerr << "CONNECTION TO PROXY FAILED....." << endl;
            close(sock);
            return sslTunnel;
        }
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        // 4. SSL Handshake WITH PROXY
        proxy_ssl = SSL_new(ctx);
        SSL_set_fd(proxy_ssl, sock);

        // [FIX]: Handshake proxy se ho raha hai, toh SNI bhi proxy ka hoga.
        // Mitmproxy local hai toh ye proxy_host (127.0.0.1) hona chahiye.
        SSL_set_tlsext_host_name(proxy_ssl, proxy_host);

        if (SSL_connect(proxy_ssl) <= 0)
        {
            char err_buff[1024];
            ERR_error_string_n(ERR_get_error(), err_buff, sizeof(err_buff));
            cerr << "Proxy SSL Handshake failed: " << err_buff << endl;
            SSL_free(proxy_ssl);
            close(sock);
            return sslTunnel;
        }

        // 5. Send CONNECT command THROUGH the Proxy SSL tunnel
        // Target domain aur port (google.com:443) yahan jayega
        char req[512];
        snprintf(req, sizeof(req),
                 "CONNECT %s:%s HTTP/1.1\r\n"
                 "Host: %s:%s\r\n\r\n",
                 domain, port, domain, port);

        SSL_write(proxy_ssl, req, (int)strlen(req));

        // 6. Check if tunnel is established
        char response[512] = {0};
        int r = SSL_read(proxy_ssl, response, sizeof(response) - 1);

        if (r <= 0 || strstr(response, "200") == nullptr)
        {
            cerr << "Proxy denied CONNECT. Response: " << response << endl;
            SSL_free(proxy_ssl);
            close(sock);
            return sslTunnel;
        }

        cout << "tunnnel is working and is connected ...." << endl;

        // Sab sahi hai, toh data bhar ke return karo
        sslTunnel.sock = sock;
        sslTunnel.proxySsl = proxy_ssl;
        return sslTunnel;
    }
};
