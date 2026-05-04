#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <chrono>
#include <string>
using namespace std;
struct TorScanModule
{
    char target[360];
    int status_code;
    char response_headers[65536];
    char reason_phrase[128];
    double latency_ms;

    TorScanModule() : status_code(0), latency_ms(0)
    {
        memset(target, 0, 360);
        memset(response_headers, 0, 65536);
        memset(reason_phrase, 0, 128);
    }
};

class Scan
{
private:
    struct timeval tv;
    SSL_CTX *ctx;
    char *target;
    char *tor_ip;
    int tor_port;
    int cp_tor_port;
    char *password;
    char *port;
    int sock;

public:
    Scan(int timeout, char *_target, char *_tor_ip, char *_port, int _tor_port, int _cp_tor_port, char *_password)
    {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        // 1. Allocate memory for pointers BEFORE copying data
        target = new char[256];
        tor_ip = new char[256];
        password = new char[2048];
        port = new char[128];

        // 2. Perform the copy safely
        strncpy(target, _target, 255);
        target[255] = '\0'; // Ensure null-termination

        strncpy(tor_ip, _tor_ip, 255);
        tor_ip[255] = '\0';

        strncpy(password, _password, 2047);
        password[2047] = '\0';

        strncpy(port, _port, 127);
        port[127] = '\0';

        this->cp_tor_port = _cp_tor_port;
        this->tor_port = _tor_port;
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        ctx = SSL_CTX_new(TLS_client_method());
    }
    ~Scan()
    {
        delete[] target;
        delete[] tor_ip;
        delete[] password;
        delete[] port;
        if (ctx)
            SSL_CTX_free(ctx);
        ctx = nullptr;
    }

    void ExecuteScan(char target_ip[256])
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(tor_port);
        inet_pton(AF_INET, tor_ip, &serv_addr.sin_addr);

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("Connection Error");
        }
        char handshake[] = {0x05, 0x01, 0x00};
        send(sock, handshake, 3, 0);
        uint8_t response[10];
        memset(response, 0, 10);
        recv(sock, response, 10, 0);
        if (response[0] == 0x05 && response[1] == 0x00)
        {
            vector<uint8_t> greeting;
            greeting.push_back(0x05); // socks version :- 5
            greeting.push_back(0x01); // cmd connect
            greeting.push_back(0x00); // rsv
            greeting.push_back(0x01); // atyp :- IPv4
            // IP target push
            struct in_addr addr;
            inet_pton(AF_INET, target_ip, &addr);
            uint8_t *ip_bytes = (uint8_t *)&addr.s_addr;
            greeting.push_back(ip_bytes[0]);
            greeting.push_back(ip_bytes[1]);
            greeting.push_back(ip_bytes[2]);
            greeting.push_back(ip_bytes[3]);

            // Sahi tarika: direct byte access
            uint16_t port_val = htons(atoi(port));
            uint8_t port_bytes[2];
            memcpy(port_bytes, &port_val, 2);

            greeting.push_back(port_bytes[0]);
            greeting.push_back(port_bytes[1]);
            uint8_t res[10];
            memset(res, 0, 10);
            send(sock, greeting.data(), greeting.size(), 0);
            int total_received = 0;
            while (total_received < 2)
            { // Hamein kam se kam 2 bytes chahiye (VER + METHOD)
                int bytes = recv(sock, res + total_received, 2 - total_received, 0);
                if (bytes <= 0)
                    break; // Error handle karo
                total_received += bytes;
            }
            if (res[0] == 0x05 && res[1] == 0x00)
            {
                uint8_t flush_buffer[8];
                recv(sock, flush_buffer, 8, 0);
                cout << "[SUCCESS]" << "You have successfully opened tunnel lets goooo boss you succeed" << endl;
            }
            else if (res[0] == 0x05 && res[1] == 0x01)
            {
                cout << "general SOCKS server failure" << endl;
                cout << (int)res[0] << " " << (int)res[1] << endl;
                TorRotation();
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x02)
            {
                cout << "connection not allowed by rulese" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                TorRotation();
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x03)
            {
                cout << "Network unreachable" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                TorRotation();
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x04)
            {
                cout << "Host unreachable " << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                TorRotation();
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x05)
            {
                cout << "Connection refused" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                TorRotation();
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x06)
            {
                cout << "TTL expired" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                TorRotation();
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x07)
            {
                cout << "Command not supported" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                TorRotation();
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x08)
            {
                cout << "Address type not supported" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                TorRotation();
                sleep(10);
            }
            else if (res[0] == 0x05 && res[1] == 0x09)
            {
                cout << "to X’FF’ unassigned" << endl;
                cout << (int)res[0] << (int)res[1] << endl;
                TorRotation();
                sleep(10);
            }
            else
            {
                cout << (int)res[0] << (int)res[1] << endl;
                TorRotation();
                sleep(10);
            }
        }
        else
        {
            cout << "[ERROR]" << "Your connection got refused or somethinng error happened" << (int)response[1] << endl;
        }
    }
    void TorRotation()
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(cp_tor_port);
        inet_pton(AF_INET, tor_ip, &serv_addr.sin_addr);
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("Control Port connection failed");
        }
        char buff[256];
        string auth = string("AUTHENTICATE \"") + password + "\"\r\n";
        send(sock, auth.c_str(), auth.length(), 0);
        int bytes = recv(sock, buff, sizeof(buff), 0);
        if (bytes < 0)
        {
            perror("You are not receiving the bytes properly");
        }
        bool isAuth250OK = string(buff).find("250 OK") != string::npos;
        if (!isAuth250OK)
        {
            perror("Authentication failed here you have not passed proper password");
        }
        string signalNewnym = string("SIGNAL NEWNYM\r\n");
        char sigBuff[256];
        send(sock, signalNewnym.c_str(), signalNewnym.length(), 0);
        int bytesRecieved = recv(sock, sigBuff, sizeof(sigBuff), 0);
        cout << "Getting a sleep coz tor is circuits are rotating......" << endl;
        sleep(10);
        cout << "I AM AWAKE lets see if this has done or not" << endl;
        if (bytesRecieved < 0)
        {
            perror("You are not recieving the bytes properly");
        }

        bool isSig250OK = string(buff).find("250 OK") != string::npos;
        if (isSig250OK)
        {
            cout << "You have passed this phase and this has done now wait for seconds and then scan again" << endl;
        }
    }

    TorScanModule *MainScan(char target_ip[256], char path[128], bool *cancel_flag)
    {
        if (cancel_flag && *cancel_flag)
        {
            return nullptr;
        }
        ExecuteScan(target_ip);
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        if (sock == -1)
        {
            return nullptr;
        }
        TorScanModule *scanModule = new TorScanModule();
        snprintf(scanModule->target, sizeof(scanModule->target), "%s%s", target, path);

        if (cancel_flag && *cancel_flag)
        {
            if (sock)
                close(sock);
            return scanModule;
        }

        bool isHttps = (strcmp(port, "443") == 0 || strcmp(port, "https") == 0);
        SSL *ssl = nullptr;
        auto start = chrono::high_resolution_clock::now();
        if (isHttps)
        {
            ssl = SSL_new(ctx);

            SSL_set_fd(ssl, sock);
            SSL_set_tlsext_host_name(ssl, target);

            int ret = SSL_connect(ssl);
            if (ret <= 0)
            {
                scanModule->status_code = -2; // TLS Handshake Failed
                SSL_free(ssl);
                close(sock);
                return scanModule;
            }
        }
        if (cancel_flag && *cancel_flag)
        {
            if (ssl)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            if (sock >= 0)
            {
                close(sock);
            }
            return scanModule;
        }
        char request[512];
        // Purana wala solid tha:
        int req_len = snprintf(request, sizeof(request),
                               "GET %s HTTP/1.1\r\n"
                               "Host: %s\r\n"
                               "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0\r\n"
                               "\r\n",
                               path, target);

        int written = (isHttps) ? SSL_write(ssl, request, req_len) : send(sock, request, req_len, 0);
        if (written <= 0)
        {
            scanModule->status_code = -60;
            if (isHttps)
                SSL_free(ssl);
            close(sock);

            return scanModule;
        }
        if (cancel_flag && *cancel_flag)
        {
            if (ssl)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            if (sock >= 0)
                close(sock);

            return scanModule;
        }
        char buff[65536];
        int total_recieved = 0;
        while (total_recieved < (int)sizeof(buff) - 1)
        {
            int bytes = (isHttps) ? SSL_read(ssl, buff + total_recieved, sizeof(buff) - total_recieved - 1)
                                  : recv(sock, buff + total_recieved, sizeof(buff) - total_recieved - 1, 0);
            if (bytes > 0)
            {
                total_recieved += bytes;
                buff[total_recieved] = '\0';

                // Yahan check kar: \r\n\r\n mila?
                char *divider = strstr(buff, "\r\n\r\n");
                if (divider != nullptr)
                {
                    // Is pointer ke aage ka sab garbage hai, delete mar!
                    *(divider + 2) = '\0';                 // \r\n\r\n ke sirf 2 character baad null terminate kar (ya +4 pura header rakhne ke liye)
                    total_recieved = (divider - buff) + 2; // Sirf header ka size update kar
                    break;                                 // Body download hone ka wait mat kar, nikal yahan se!
                }
            }
            else
            {
                int err = (isHttps) ? SSL_get_error(ssl, bytes) : errno;
                // Agar SSL_ERROR_WANT_READ hai, toh thoda wait karke retry kar
                if (isHttps && (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE))
                {
                    usleep(10000); // 10ms wait
                    continue;
                }
                break; // Error ya connection close
            }
        }

        auto end = chrono::high_resolution_clock::now();
        if (total_recieved > 0)
        {
            buff[total_recieved] = '\0';
            // Safe copy to struct
            strncpy(scanModule->response_headers, buff, sizeof(scanModule->response_headers) - 1);

            // 1. Buff mein "HTTP/" dhoondho
            char *http_ptr = strstr(buff, "HTTP/");

            if (http_ptr != nullptr)
            {
                scanModule->status_code = atoi(http_ptr + 9);
            }
            else
            {
                scanModule->status_code = -80;
            }
            char *line_end = strpbrk(buff, "\r\n");
            if (line_end != nullptr)
            {
                *line_end = '\0'; // Line ko kaat do
                strncpy(scanModule->reason_phrase, buff, sizeof(scanModule->reason_phrase) - 1);
            }
            if (cancel_flag && *cancel_flag)
            {
                if (ssl)
                {
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
                }
                if (sock >= 0)
                    close(sock);
                return scanModule;
            }
        }
        else
        {
            // Agar total_received == 0, toh already -70 set hai (ya error code)
            if (scanModule->status_code == 0)
            {
                scanModule->status_code = -70;
            }
        }
        if (cancel_flag && *cancel_flag)
        {
            if (ssl)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            if (sock >= 0)
                close(sock);
            return scanModule;
        }
        scanModule->latency_ms = chrono::duration<double, milli>(end - start).count();
        if (cancel_flag && *cancel_flag)
        {
            if (ssl)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            if (sock >= 0)
                close(sock);
            return scanModule;
        }
        if (isHttps)
        {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        close(sock);

        return scanModule;
    }
};

string WebToIp(char *target)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    if (getaddrinfo(target, NULL, &hints, &res) != 0)
        return "";

    char ip_str[INET_ADDRSTRLEN];
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str, INET_ADDRSTRLEN);

    freeaddrinfo(res);     // LEAK FIXED!
    return string(ip_str); // Stack se copy return ho raha hai, SAFE!
}

extern "C"
{
    void *CreateEngine(char *target, char *port, char *tor_ip, char *password, int tor_port, int cp_tor_port, int timeout)
    {
        return new Scan(timeout, target, tor_ip, port, tor_port, cp_tor_port, password);
    }
    TorScanModule *EngineScan(void *engine, char path[128], bool *cancel_flag, char *target)
    {
        char ip_str[INET_ADDRSTRLEN] = {0};
        string ip = WebToIp(target);
        if (ip.empty())
        {
            return nullptr;
        }
        strncpy(ip_str, ip.c_str(), INET_ADDRSTRLEN - 1);
        return static_cast<Scan *>(engine)->MainScan(ip_str, path, cancel_flag);
    }
    void DestroyResult(TorScanModule *res)
    {
        if (res)
        {
            delete res;
        }
    }
    void DestroyEngine(void *engine)
    {
        if (engine)
        {
            delete static_cast<Scan *>(engine);
        }
    }
    void RotateTorCircuits(void *engine)
    {
        static_cast<Scan *>(engine)->TorRotation();
    }
}
