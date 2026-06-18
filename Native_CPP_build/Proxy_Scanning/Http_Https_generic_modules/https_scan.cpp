#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "scan_model.cpp"
#include <sstream>
using namespace std;

class GenericHttpsScan
{
private:
    char domain[256];
    char headers[8192];

public:
    GenericHttpsScan(char _domain[256], char _headers[8192])
    {
        strncpy(domain, _domain, 256);
        strncpy(headers, _headers, 8192);
        SSL_library_init();
        OpenSSL_add_all_algorithms();
    }

    ScanOutput scan(char path[2048], int &sock, SSL *&target_ssl)
    {
        ScanOutput result;
        memset(&result, 0, sizeof(result)); // Clear memory safely
        snprintf(result.domain, sizeof(result.domain), "%s%s", domain, path);

        std::string clean_headers(headers);

        // 2. Look for explicit literal "\\r\\n" text strings if they snuck in from P/Invoke
        size_t pos;
        while ((pos = clean_headers.find("\\r\\n")) != std::string::npos)
        {
            clean_headers.replace(pos, 4, "\r\n");
        }

        // 3. Fix any lone '\n' characters that are missing a companion '\r'
        std::string sanitized = "";
        std::string line;
        std::stringstream ss(clean_headers);
        while (std::getline(ss, line))
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

        char req[10240];
        // Ensure raw headers are formatted perfectly before hitting the wire
        int req_len = snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s\r\n%s\r\n", path, domain, sanitized.c_str());

        int bytes_written = SSL_write(target_ssl, req, req_len);
        if (bytes_written <= 0)
        {
            SSL_free(target_ssl);
            close(sock);
            return result;
        }

        char buff[65536];
        memset(buff, 0, sizeof(buff));
        int total_received = 0;

        while (total_received < (int)sizeof(buff) - 1)
        {
            int bytes_to_read = (int)sizeof(buff) - total_received - 1;
            int bytes = SSL_read(target_ssl, buff + total_received, bytes_to_read);

            if (bytes <= 0)
                break; // Clean handle for disconnect or error

            total_received += bytes;
            buff[total_received] = '\0';

            // Search the entire accumulated buffer safely without string mutation
            char *divider = strstr(buff, "\r\n\r\n");
            if (divider != nullptr)
            {
                // Copy everything up to and including the header boundary safely
                size_t header_len = (divider + 4) - buff;
                if (header_len < sizeof(result.headers))
                {
                    memcpy(result.headers, buff, header_len);
                    result.headers[header_len] = '\0';
                }
                break;
            }
        }
        return result;
    }
};
