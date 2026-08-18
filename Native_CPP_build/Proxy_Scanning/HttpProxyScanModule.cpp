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
#include "Proxy_Scan.cpp"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <chrono>
#include "../Generic_Module/interface_scan_module.cpp"
#include "../Generic_Module/output_struct.cpp"
#include "../Generic_Module/wires.cpp"
struct HttpScanOutput
{
    char domain[3072];
    char headers[65536];
    char reason_phrase[128];
    int status_code;
    double latency_ms;
};

class HttpProxyScan
{
private:
    char domain[256];
    char headers[8192];
    char proxy_host[256];
    char proto_port[128];
    int proxy_port;
    int timeout;
    SSL_CTX *ctx;
    SSL *ssl;

public:
    HttpProxyScan(char _domain[256], char _proto_port[128], char _headers[8192], char _proxy_host[256], int _proxy_port, int _timeout)
    {
        strncpy(domain, _domain, 256);
        strncpy(headers, _headers, 8192);
        strncpy(proxy_host, _proxy_host, 256);
        strncpy(proto_port, _proto_port, 128);
        proxy_port = _proxy_port;
        timeout = _timeout;
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        ctx = SSL_CTX_new(TLS_client_method());
    }

    ~HttpProxyScan()
    {
        if (ctx)
        {
            SSL_CTX_free(ctx);
        }
    }

    ProxyScanOutputModel scan(char path[2048], bool *cancel_flag)
    {
        ProxyScanOutputModel result;
        if (cancel_flag && *cancel_flag)
        {
            return result;
        }
        GenericStruct output;
        GenericInterface scanInterface(domain, headers, proto_port);
        ProxyScan scan(domain, proxy_port, proxy_host, proto_port, timeout);
        auto start = chrono::high_resolution_clock::now();
        int sock = scan.HttpProxy();
        bool isHttps = (strcmp(proto_port, "443") == 0);
        if (isHttps)
        {
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, sock);
            SSL_set_tlsext_host_name(ssl, domain);
            if (SSL_connect(ssl) < 0)
            {
                SSL_free(ssl);
                close(sock);
                return result;
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
            return result;
        }
        output = scanInterface.interface_scan(path, cancel_flag, sock, ssl);
        if (cancel_flag && *cancel_flag)
        {
            if (isHttps)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            close(sock);
            return result;
        }
        if (sock < 0)
        {
            if (isHttps)
            {
                if (ssl == nullptr)
                {
                    return result;
                }
            }
            return result;
        }
        if (cancel_flag && *cancel_flag)
        {
            if (isHttps)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            close(sock);
            return result;
        }
        auto end = chrono::high_resolution_clock::now();
        strncpy(result.domain, output.domain, 3072);
        strncpy(result.headers, output.headers, 65536);
        result.status_code = extract_status_from_buffer(output.headers);
        if (cancel_flag && *cancel_flag)
        {
            if (isHttps)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            close(sock);
            return result;
        }
        char *line_end = strpbrk(output.headers, "\r\n");
        if (line_end != nullptr)
        {
            *line_end = '\0';
            strncpy(result.reason_phrase, output.headers, sizeof(result.reason_phrase) - 1);
        }
        result.latency_ms = chrono::duration<double, milli>(end - start).count();
        if (cancel_flag && *cancel_flag)
        {
            if (isHttps)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            close(sock);
            return result;
        }
        if (isHttps && ssl)
        {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            if (sock >= 0)
            {
                close(sock);
            }
        }
        if (sock >= 0)
        {
            close(sock);
        }
        return result;
    }
};

extern "C"
{
    void *http_create(char domain[256], char proto_port[128], char headers[8192], char proxy_host[256], int timeout, int proxy_port)
    {
        return new HttpProxyScan(domain, proto_port, headers, proxy_host, proxy_port, timeout);
    }

    ProxyScanOutputModel http_scan(void *engine, char path[2048], bool *cancel_flag)
    {
        return static_cast<HttpProxyScan *>(engine)->scan(path, cancel_flag);
    }

    void http_engine_destroy(void *engine)
    {
        delete static_cast<HttpProxyScan *>(engine);
    }
}
