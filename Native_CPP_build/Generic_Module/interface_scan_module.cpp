#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include "output_struct.cpp"
#include <cstring>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <chrono>
#include <sstream>
#include <string>
using namespace std;

class GenericInterface
{
private:
    char domain[256];
    char proto_port[128];
    char headers[8192];

public:
    GenericInterface(char _domain[256], char _headers[8192], char _proto_port[128])
    {
        strncpy(domain, _domain, 256);
        strncpy(headers, _headers, 8192);
        strncpy(proto_port, _proto_port, 128);
    }
    
    GenericStruct interface_scan(char path[2048], bool *cancel_flag, int &sock, SSL *&ssl)
    {
        bool isHttps = (strcmp(proto_port, "443") == 0);
        string clean_headers(headers);
        size_t pos;
        while ((pos = clean_headers.find("\\r\\n")) != string::npos)
        {
            clean_headers.replace(pos, 4, "\r\n");
        }

        string sanitized = "";
        string line;
        stringstream ss(clean_headers);
        while (getline(ss, line))
        {
            while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' '))
            {
                line.pop_back();
            }
            if (!line.empty())
            {
                sanitized += line + "\r\n";
            }
        }
        
        if (cancel_flag && *cancel_flag)
        {
            if (isHttps && ssl) { SSL_shutdown(ssl); SSL_free(ssl); ssl = nullptr; }
            if (sock >= 0) { close(sock); sock = -1; }
            return GenericStruct();
        }
        
        GenericStruct scanResult;
        memset(&scanResult, 0, sizeof(GenericStruct));
        
        char req[10240];
        // FIX: Using the sanitized.c_str() which has correct \r\n padding layouts
        int req_len = snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s\r\n%s\r\n", path, domain, sanitized.c_str());
        snprintf(scanResult.domain, sizeof(scanResult.domain), "%s%s", domain, path);
        
        if (cancel_flag && *cancel_flag)
        {
            if (isHttps && ssl) { SSL_shutdown(ssl); SSL_free(ssl); ssl = nullptr; }
            if (sock >= 0) { close(sock); sock = -1; }
            return GenericStruct();
        }
        
        int written = (isHttps) ? SSL_write(ssl, req, req_len) : send(sock, req, req_len, 0);
        if (written <= 0)
        {
            if (isHttps && ssl) { SSL_shutdown(ssl); SSL_free(ssl); ssl = nullptr; }
            if (sock >= 0) { close(sock); sock = -1; }
            return scanResult;
        }
        
        if (cancel_flag && *cancel_flag)
        {
            if (isHttps && ssl) { SSL_shutdown(ssl); SSL_free(ssl); ssl = nullptr; }
            if (sock >= 0) { close(sock); sock = -1; }
            return GenericStruct();
        }
        
        char buff[65536];
        memset(buff, 0, sizeof(buff));
        int total_recieved = 0;
        
        while (total_recieved < (int)sizeof(buff) - 1)
        {
            if (cancel_flag && *cancel_flag)
            {
                if (isHttps && ssl) { SSL_shutdown(ssl); SSL_free(ssl); ssl = nullptr; }
                if (sock >= 0) { close(sock); sock = -1; }
                return GenericStruct();
            }
            
            int bytes_to_read = (int)sizeof(buff) - total_recieved - 1;
            
            // FIX: SSL_read for HTTPS, recv for cleartext HTTP!
            int bytes = (isHttps) ? SSL_read(ssl, buff + total_recieved, bytes_to_read)
                                  : recv(sock, buff + total_recieved, bytes_to_read, 0);

            if (bytes <= 0)
            { 
                if (isHttps && ssl) {
                    int err = SSL_get_error(ssl, bytes);
                    if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
                        usleep(10000);
                        continue;
                    }
                }
                break;
            }

            total_recieved += bytes;
            buff[total_recieved] = '\0';

            char *divider = strstr(buff, "\r\n\r\n");
            if (divider != nullptr)
            {
                size_t header_len = (divider + 4) - buff;
                if (header_len < sizeof(scanResult.headers))
                {
                    memcpy(scanResult.headers, buff, header_len);
                    scanResult.headers[header_len] = '\0';
                }
                break;
            }
        }

        return scanResult;
    }
};