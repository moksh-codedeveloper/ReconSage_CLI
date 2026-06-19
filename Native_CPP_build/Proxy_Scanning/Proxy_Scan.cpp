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
            close(sock);
            return -1;
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
            greeting.push_back(0x05); // SOCKS version 5
            greeting.push_back(0x01); // CMD: CONNECT
            greeting.push_back(0x00); // RSV
            greeting.push_back(0x03); // ATYPE: Domain Name
            greeting.push_back((uint8_t)target_len);
            for (size_t i = 0; i < target_len; i++)
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
                // ==================== FIX: STREAM DRAINER FOR TOR DYNAMIC RESPONSES ====================
                uint8_t atype_block[2];
                // RSV aur ATYPE fields read karo
                recv(sock, atype_block, 2, 0); 
                
                uint8_t atype = atype_block[1];
                int remaining_payload_size = 0;

                if (atype == 0x01) {       // Tor returned an IPv4 map back
                    remaining_payload_size = 4 + 2; 
                } 
                else if (atype == 0x03) {  // Tor returned a Domain layout tracking string back
                    uint8_t domain_len = 0;
                    recv(sock, &domain_len, 1, 0); 
                    remaining_payload_size = domain_len + 2; 
                } 
                else if (atype == 0x04) {  // Tor returned IPv6 maps back
                    remaining_payload_size = 16 + 2; 
                }

                // Pure residual network bytes flush operation
                if (remaining_payload_size > 0) {
                    vector<char> flush_buffer(remaining_payload_size);
                    int bytes_drained = 0;
                    while (bytes_drained < remaining_payload_size) {
                        int r = recv(sock, flush_buffer.data() + bytes_drained, remaining_payload_size - bytes_drained, 0);
                        if (r <= 0) break;
                        bytes_drained += r;
                    }
                }
                // =======================================================================================
                return sock;
            }
            else if (res[0] == 0x05 && res[1] == 0x01)
            {
                cout << "general SOCKS server failure" << endl;
                close(sock);
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x02)
            {
                cout << "connection not allowed by rulese" << endl;
                close(sock);
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x03)
            {
                cout << "Network unreachable" << endl;
                close(sock);
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x04)
            {
                cout << "Host unreachable " << endl;
                close(sock);
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x05)
            {
                cout << "Connection refused" << endl;
                close(sock);
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x06)
            {
                cout << "TTL expired" << endl;
                close(sock);
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x07)
            {
                cout << "Command not supported" << endl;
                close(sock);
                return -1;
            }
            else if (res[0] == 0x05 && res[1] == 0x08)
            {
                cout << "Address type not supported" << endl;
                close(sock);
                return -1;
            }
            else
            {
                cout << "Unassigned error packet trace: " << (int)res[0] << " " << (int)res[1] << endl;
                close(sock);
                return -1;
            }
        }
        else
        {
            cout << "[ERROR] Connection refused by proxy header layer checkpoint: " << (int)response[1] << endl;
            close(sock);
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
        SSL_Tunnel sslTunnel; 
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
            return sslTunnel;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(proxy_port);
        inet_pton(AF_INET, proxy_host, &addr.sin_addr);

        if (connect(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
        {
            cerr << "CONNECTION TO PROXY FAILED....." << endl;
            close(sock);
            return sslTunnel;
        }
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        proxy_ssl = SSL_new(ctx);
        SSL_set_fd(proxy_ssl, sock);
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

        char req[512];
        snprintf(req, sizeof(req),
                 "CONNECT %s:%s HTTP/1.1\r\n"
                 "Host: %s:%s\r\n\r\n",
                 domain, port, domain, port);

        SSL_write(proxy_ssl, req, (int)strlen(req));

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

        sslTunnel.sock = sock;
        sslTunnel.proxySsl = proxy_ssl;
        return sslTunnel;
    }
};