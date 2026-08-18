/*
 * ReconSage_Cli - Advanced Network & Telemetry Reconnaissance Framework
 * Copyright (C) 2026 ReconSage_Cli Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
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
#include <cstring>
#include "../Generic_Module/interface_scan_module.cpp"
#include "../Generic_Module/SocksModule.cpp"
#include "../Generic_Module/wires.cpp"

using namespace std;

class Scan
{
private:
    char domain[256];
    char proto_port[128];
    char headers[8192];
    char tor_ip[256];
    int tor_port;
    int timeout;
    char password[8192];
    int cp_tor_port;
    SSL_CTX *ctx;

public:
    Scan(char _domain[256], char _proto_port[128], char _headers[8192], char _tor_ip[256], char _password[8192], int _timeout, int _tor_port, int _cp_tor_port)
    {
        strncpy(domain, _domain, 256);
        strncpy(proto_port, _proto_port, 128);
        strncpy(headers, _headers, 8192);
        strncpy(tor_ip, _tor_ip, 256);
        strncpy(password, _password, 8192);
        tor_port = _tor_port;
        timeout = _timeout;
        cp_tor_port = _cp_tor_port;

        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        ctx = SSL_CTX_new(TLS_client_method());
    }

    // FIX 3: Added Missing Destructor to prevent OpenSSL context leak
    ~Scan()
    {
        if (ctx)
        {
            SSL_CTX_free(ctx);
            ctx = nullptr;
        }
    }

    int TorRotation()
    {
        int ctrl_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (ctrl_sock < 0)
            return -1;

        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(cp_tor_port);
        inet_pton(AF_INET, tor_ip, &serv_addr.sin_addr);

        if (connect(ctrl_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("Control Port connection failed");
            close(ctrl_sock);
            return -1;
        }

        char buff[256] = {0};
        string auth = string("AUTHENTICATE \"") + password + "\"\r\n";
        send(ctrl_sock, auth.c_str(), auth.length(), 0);
        int bytes = recv(ctrl_sock, buff, sizeof(buff) - 1, 0);

        if (bytes < 0 || string(buff).find("250 OK") == string::npos)
        {
            perror("Tor Control Authentication failed");
            close(ctrl_sock);
            return -1;
        }

        string signalNewnym = string("SIGNAL NEWNYM\r\n");
        send(ctrl_sock, signalNewnym.c_str(), signalNewnym.length(), 0);
        memset(buff, 0, sizeof(buff));
        recv(ctrl_sock, buff, sizeof(buff) - 1, 0);

        cout << "[TOR] Tor circuits rotating safely..." << endl;
        sleep(10); // Cool down wait for circuit build

        close(ctrl_sock); // Close control connection cleanly
        return 0;
    }

    ScanOutputStruct tor_scan(char path[2048], bool *cancel_flag)
    {
        SocksProxy tunnel(domain, tor_ip, timeout, tor_port, atoi(proto_port));
        SSL *ssl = nullptr;
        ScanOutputStruct output;
        memset(&output, 0, sizeof(ScanOutputStruct));

        GenericInterface interface(domain, headers, proto_port);
        if (cancel_flag && *cancel_flag)
            return output;

        bool isHttps = (strcmp(proto_port, "443") == 0);
        auto start = chrono::high_resolution_clock::now();

        int sock = tunnel.SockTunnel();
        if (sock < 0)
            return output;

        if (isHttps)
        {
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, sock);
            SSL_set_tlsext_host_name(ssl, domain);
            if (SSL_connect(ssl) <= 0)
            {
                SSL_free(ssl);
                close(sock);
                return output;
            }
        }

        if (cancel_flag && *cancel_flag)
        {
            if (isHttps && ssl)
            {
                SSL_free(ssl);
            }
            close(sock);
            return output;
        }

        GenericStruct result = interface.interface_scan(path, cancel_flag, sock, ssl);
        int status_code = extract_status_from_buffer(result.headers);

        // FIX 2: Corrected validation boundary for error detection
        if (status_code < 200 || (status_code >= 400 && status_code != 404))
        {
            cout << "[WARNING C++] Bad status target match [" << status_code << "]. Resetting transport line..." << endl;
            if (isHttps && ssl)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
                ssl = nullptr;
            }
            if (sock >= 0)
            {
                close(sock);
                sock = -1;
            }
            // Trigger raw control circuit rotation
            TorRotation();
            usleep(10000);
            if (cancel_flag && *cancel_flag)
                return output;

            // FIX 1: Re-establish pristine tunnel connection and re-run query execution
            sock = tunnel.SockTunnel();
            if (sock >= 0)
            {
                if (isHttps)
                {
                    ssl = SSL_new(ctx);
                    SSL_set_fd(ssl, sock);
                    SSL_set_tlsext_host_name(ssl, domain);
                    if (SSL_connect(ssl) <= 0)
                    {
                        SSL_free(ssl);
                        close(sock);
                        return output;
                    }
                }
            }
            result = interface.interface_scan(path, cancel_flag, sock, ssl);
            status_code = extract_status_from_buffer(result.headers);
        }

        auto end = chrono::high_resolution_clock::now();
        output.latency_ms = chrono::duration<double, milli>(end - start).count();

        strncpy(output.headers, result.headers, sizeof(output.headers) - 1);
        strncpy(output.domain, result.domain, sizeof(output.domain) - 1);
        output.status_code = status_code;

        char *line_end = strpbrk(result.headers, "\r\n");
        if (line_end != nullptr)
        {
            *line_end = '\0';
            strncpy(output.reason_phrase, result.headers, sizeof(output.reason_phrase) - 1);
        }

        if (isHttps && ssl)
        {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        if (sock >= 0)
            close(sock);

        return output;
    }
};

extern "C"
{
    void *create_engine(char domain[256], char proto_port[128], char headers[8192], char tor_ip[256], char password[8192], int timeout, int tor_port, int cp_tor_port)
    {
        return new Scan(domain, proto_port, headers, tor_ip, password, timeout, tor_port, cp_tor_port);
    }
    ScanOutputStruct tor_scan_engine(char path[2048], void *engine, bool *cancel_flag)
    {
        if (!engine)
            return ScanOutputStruct();
        return static_cast<Scan *>(engine)->tor_scan(path, cancel_flag);
    }
    void destroy_tor_engine(void *engine)
    {
        if (engine)
        {
            delete static_cast<Scan *>(engine);
        }
    }
}