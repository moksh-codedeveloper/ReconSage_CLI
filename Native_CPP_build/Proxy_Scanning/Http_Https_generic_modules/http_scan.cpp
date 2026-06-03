#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<cstring>
#include<cstdio>
#include<unistd.h>
using namespace std;

struct generic_output{
    char domain[3096];
    char headers[65536];
};

class GenericHttpScan{
    private:
        char domain[256];
        char headers[8192];
    public:
        GenericHttpScan(char _domain[256], char _proto_port[128], char _headers[8192], char _proxy_host[256], int _proxy_port, int _timeout){
            strncpy(domain, _domain, 256);
            strncpy(headers, _headers, 8192);
        }
        generic_output http_scan(char path[2048], int &sock){
            generic_output output;
            snprintf(output.domain, sizeof(output.domain), "%s%s", domain, path);
            char req[10240];
            int req_len = snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s\r\n%s", path, domain, headers);
            int total_received = 0;
            char buff[65536];
            int byte_received = send(sock, req, req_len, 0);
            if(byte_received <= 0){
                close(sock);
                return output;
            }
            while(total_received < (int)sizeof(buff) - 1){
                    int bytes_to_read = (int)sizeof(buff) - total_received - 1;
                    int bytes = recv(sock, buff + total_received, bytes_to_read, 0);
                    if(bytes == 0) break;
                    if(bytes < 0){
                        close(sock);
                        break;
                    }
                    total_received += bytes;
                    buff[total_received] = '\0';

                    char *divider = strstr(buff, "\r\n\r\n");
                    if(divider != nullptr){
                        *(divider + 2) = '\0';
                        total_received = (divider - buff) + 2;
                        break;
                    }
            }
            if(total_received > 0){
                strncpy(output.headers, buff, sizeof(output.headers) - 1);
            }
            return output;
        }
};
