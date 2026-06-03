#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<openssl/ssl.h>
#include<openssl/err.h>
#include"scan_model.cpp"
using namespace std;

class GenericHttpsScan{
    private:
        char domain[256];
        char headers[8192];
    public:
        GenericHttpsScan(char _domain[256], char _headers[8192]){
            strncpy(domain, _domain, 256);
            strncpy(headers, _headers, 8192);
            SSL_library_init();
            OpenSSL_add_all_algorithms();
        }

        ScanOutput scan(char path[2048], int &sock, SSL *&target_ssl){
            ScanOutput result;
            snprintf(result.domain, sizeof(result.domain), "%s%s", domain, path);
            char req[10240];
            int req_len = snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s\r\n%s", path, domain, headers);
            int bytes_written = SSL_write(target_ssl, req, req_len);
            if(bytes_written <= 0){
                SSL_free(target_ssl);
                close(sock);
                return result;
            } else {
                char buff[65536];
                int total_recieved = 0;
                while (total_recieved < (int)sizeof(buff) - 1)
                {
                    int bytes_to_read = (int)sizeof(buff) - total_recieved - 1;
                    int bytes = SSL_read(target_ssl, buff + total_recieved, bytes_to_read);
                    if (bytes == 0) break;
                    if (bytes < 0) break;
                    total_recieved += bytes;
                    buff[total_recieved] = '\0';

                    char *divider = strstr(buff, "\r\n\r\n");
                    if (divider != nullptr)
                    {
                        *(divider + 2) = '\0';
                        total_recieved = (divider - buff) + 2;
                        break; // headers done, stop reading body
                    }
                }
                if(total_recieved > 0){
                    strncpy(result.headers, buff, sizeof(result.headers) - 1);
                }
            }
            return result;
        }
};
