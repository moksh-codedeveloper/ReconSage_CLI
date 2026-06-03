#include<iostream>
#include<cstring>
#include<sys/socket.h>
#include "Proxy_Scan.cpp"
#include<chrono>
#include<unistd.h>
#include<arpa/inet.h>

using namespace std;

struct ProxyOutput{
    char response_headers[65536];
    char domain[3096];
    char reason_phrase[128];
    int status_code;
    double latency_ms;
};

class HttpProxyScan{
    private:
            char domain[256];
            char proxy_host[256];
            char proto_port[128];
            char headers[5120];
            int proxy_port;
            int timeout;
    public:
        HttpProxyScan(char target[256], char _proxy_host[256], char port[128], char header[5120], int _timeout, int _proxy_port){
            strncpy(domain, target, 256);
            strncpy(proxy_host, _proxy_host, 256);
            strncpy(proto_port, port, 128);
            strncpy(headers, header, 5120);
            proxy_port = _proxy_port;
            timeout = _timeout;
        }

        int extract_status_from_buffer(const char *buffer)                                                                                                                               
        {
            if (!buffer)                                                                                                                                                                 
                return -1;                                                                                                                                                                                                                                                                                                                                                     
            const char *ptr = buffer;                                                                                                                                                    
            while (*ptr != '\0' && *ptr != ' ' && *ptr != '\r' && *ptr != '\n')                                                                                                          
            {                                                                                                                                                                            
                ptr++;                                                                                                                                                                   
            }                                                                                                                                                                            
            if (*ptr != ' ')                                                                                                                                                             
            {                                                                                                                                                                            
                return -1;                                                                                                                                                               
            }                                                                                                                                                                            
            ptr++;                                                                                                                                                                       
            int code = 0;                                                                                                                                                                
            for (int i = 0; i < 3; ++i)                                                                                                                                                  
            {                                                                                                                                                                            
                if (*ptr >= '0' && *ptr <= '9')                                                                                                                                          
                {                                                                                                                                                                        
                    code = code * 10 + (*ptr - '0');                                                                                                                                     
                    ptr++;                                                                                                                                                               
                }                                                                                                                                                                        
                else                                                                                                                                                                     
                {                                                                                                                                                                        
                    return -1;                                                                                                                                                           
                }                                                                                                                                                                        
            }                                                                                                                                                                                                                                                                                                                                                                  
            return code;                                                                                                                                                                 
        }

        ProxyOutput scan(char path[2048]){
            ProxyOutput output;
            ProxyScan proxyScan(domain, proxy_port, proxy_host, proto_port, timeout);
            snprintf(output.domain, sizeof(output.domain), "%s%s", domain, path);
            auto start = chrono::high_resolution_clock::now();
            int sock = proxyScan.HttpProxy();
            char req[8192];
            int req_len = snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s\r\n%s", path, domain, headers);
            int total_received = 0;
            char buff[65536];
            int byte_received = send(sock, req, req_len, 0);
            if( byte_received <= 0){
                output.status_code = -10;
                close(sock);
                return output;
            }
            while(total_received < (int)sizeof(buff) - 1){
                int bytes_to_read = (int)sizeof(buff) - total_received - 1;
                int bytes = recv(sock, buff + total_received, bytes_to_read, 0);
                if(bytes == 0) break;
                if(bytes < 0 )
                {
                    output.status_code = errno;
                    close(sock);
                    break;
                }
                char *divider = strstr(buff, "\r\n\r\n");
                if(divider != nullptr){
                    *(divider + 2) = '\0';
                    total_received = (divider - buff) + 2;
                    break;
                }
            }
            auto end = chrono::high_resolution_clock::now();
            if(total_received > 0){
                buff[total_received] = '\0';
                strncpy(output.response_headers, buff, sizeof(output.response_headers) - 1);
                // add extraction of status code and many more things in this part of code
                output.status_code = extract_status_from_buffer(buff);
                char *line_end = strpbrk(buff, "\r\n");
                if(line_end != nullptr){
                    *line_end = '\0';
                    strncpy(output.reason_phrase, buff, 128);
                }
            }
            output.latency_ms = chrono::duration<double, milli>(end - start).count();
            close(sock);
            return output;
        }
};

