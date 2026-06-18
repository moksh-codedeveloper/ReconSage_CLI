#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include "scan_model.cpp"
#include <sstream>
using namespace std;
class GenericHttpScan
{
private:
    char domain[256];
    char headers[8192];

public:
    GenericHttpScan(char _domain[256], char _headers[8192])
    {
        strncpy(domain, _domain, 256);
        strncpy(headers, _headers, 8192);
    }
    ScanOutput http_scan(char path[2048], int &sock)
    {
        ScanOutput output;
        memset(&output, 0, sizeof(output)); // Safely zero out struct memory
        snprintf(output.domain, sizeof(output.domain), "%s%s", domain, path);
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
        int req_len = snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s\r\n%s", path, domain, sanitized.c_str());

        int byte_received = send(sock, req, req_len, 0);
        if (byte_received <= 0)
        {
            close(sock);
            return output;
        }

        char buff[65536];
        memset(buff, 0, sizeof(buff)); // Ensure buffer is pristine
        int total_received = 0;

        while (total_received < (int)sizeof(buff) - 1)
        {
            int bytes_to_read = (int)sizeof(buff) - total_received - 1;
            int bytes = recv(sock, buff + total_received, bytes_to_read, 0);

            if (bytes <= 0)
            { // Unified catch for clean disconnect or socket errors
                close(sock);
                break;
            }

            total_received += bytes;
            buff[total_received] = '\0';

            // Look for the header termination boundary safely
            char *divider = strstr(buff, "\r\n\r\n");
            if (divider != nullptr)
            {
                // Calculate exact offset up to the end of "\r\n\r\n" (4 bytes)
                size_t header_len = (divider + 4) - buff;
                if (header_len < sizeof(output.headers))
                {
                    memcpy(output.headers, buff, header_len);
                    output.headers[header_len] = '\0'; // Properly null terminate destination string
                }
                break; // Headers completely read, break loop safely without buffer mutation
            }
        }
        return output;
    }
};
