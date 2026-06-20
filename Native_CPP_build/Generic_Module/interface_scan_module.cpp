#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include "output_struct.cpp"
#include <cstring>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <chrono>
using namespace std;

class GenericInterface
{
private:
    char domain[256];
    char proto_port[128];
    char headers[8192];

public:
    GenericInterface(char _domain[256], char _headers[65536], char _proto_port[128])
    {
        strncpy(domain, _domain, 256);
        strncpy(headers, _headers, 65536);
        strncpy(proto_port, _proto_port, 128);
    }
    GenericStruct interface_scan(char path[2048], bool *cancel_flag, int &sock, SSL *&ssl)
    {
        bool isHttps = (strcmp(proto_port, "443") == 0 || strcmp(proto_port, "https") == 0);
        // headers cleaning
        string clean_headers(headers);

        // 2. Look for explicit literal "\\r\\n" text strings if they snuck in from P/Invoke
        size_t pos;
        while ((pos = clean_headers.find("\\r\\n")) != string::npos)
        {
            clean_headers.replace(pos, 4, "\r\n");
        }

        // 3. Fix any lone '\n' characters that are missing a companion '\r'
        string sanitized = "";
        string line;
        stringstream ss(clean_headers);
        while (getline(ss, line))
        {
            // Strip off any existing trailing carriage returns, newlines, or spaces
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
            if(isHttps){
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            close(sock);
            return GenericStruct();
        }
        GenericStruct scanResult;
        char req[10240];
        int req_len = snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s\r\n%s\r\n", path, domain, headers);
        snprintf(scanResult.domain, sizeof(scanResult.domain), "%s%s", domain, path);
        if (cancel_flag && *cancel_flag)
        {
            if (isHttps)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            close(sock);
            return GenericStruct();
        }
        int written = (isHttps) ? SSL_write(ssl, req, req_len) : send(sock, req, req_len, 0);
        if (written <= 0)
        {
            if (isHttps)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            close(sock);
            return scanResult;
        }
        if (cancel_flag && *cancel_flag)
        {
            if (isHttps)
            {
                SSL_shutdown(ssl);
                SSL_free(ssl);
            }
            close(sock);
            return GenericStruct();
        }
        char buff[65536];
        memset(buff, 0, sizeof(buff));
        int total_recieved = 0;
        while (total_recieved < (int)sizeof(buff) - 1)
        {
            int bytes_to_read = (int)sizeof(buff) - total_recieved - 1;
            int bytes = (isHttps) ? recv(sock, buff + total_recieved, bytes_to_read, 0) : SSL_read(ssl, buff + total_recieved, bytes_to_read);

            if (bytes <= 0)
            { // Unified catch for clean disconnect or socket errors
                close(sock);
                break;
            }

            total_recieved += bytes;
            buff[total_recieved] = '\0';

            // Look for the header termination boundary safely
            char *divider = strstr(buff, "\r\n\r\n");
            if (divider != nullptr)
            {
                // Calculate exact offset up to the end of "\r\n\r\n" (4 bytes)
                size_t header_len = (divider + 4) - buff;
                if (header_len < sizeof(scanResult.headers))
                {
                    memcpy(scanResult.headers, buff, header_len);
                    scanResult.headers[header_len] = '\0'; // Properly null terminate destination string
                }
                break;
            }
        }
        return scanResult;
    }
};