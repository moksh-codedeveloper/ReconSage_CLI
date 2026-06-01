#include"Proxy_Scan.cpp"
#include<chrono>
using namespace std;

struct SocksOutput{
    char domain[3072];
    double latency_ms;
    int status_code;
    char response_headers[65536];
    char reason_phrase[128];
};

class SocksScan{
private:
    char domain[256];
    char proto_port[128];
    int proxy_port;
    char proxy_host[256];
    char headers[5120];
    int timeout;
public:
    SocksScan(char _domain[256], char _proto_port[128], char _proxy_host[256], char _headers[5120], int _proxy_port, int _timeout){
        strncpy(domain, _domain, 256);
        strncpy(proto_port, _proto_port, 128);
        strncpy(proxy_host, _proxy_host, 256);
        proxy_port = _proxy_port;
        timeout = _timeout;
        strncpy(headers, _headers, 5120);
    }
    int extract_status_from_buffer(char buff[65536]){
        if(!buff){
            return -1;
        }
        const char *ptr = buff;
        while(*ptr != '\0' && *ptr != ' ' && *ptr != '\r' && *ptr != '\n'){
            ptr++;
        }
        if(*ptr != ' '){
            return -1;
        }

        ptr++;
        int code = 0;
        for(int i = 0; i < 3; ++i){
            if(*ptr >= '0' && *ptr <= '9'){
                code = code * 10 + (*ptr - '0');
                ptr++;
            } else {
                return -1;
            }
        }
        return code;
    }
    SocksOutput scan(char path[2048]){
        SocksOutput output;
        ProxyScan proxyScan(domain, proxy_port, proxy_host, proto_port, timeout);
        snprintf(output.domain, sizeof(output.domain), "%s%s", domain, path);
        auto start = chrono::high_resolution_clock::now();
        int sock = proxyScan.SocksTunnel();
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

int main(){
    char domain[] = "www.example.com";
    char proxy_host[] = "127.0.0.1";
    char port[] = "80";
    int proxy_port = 1080;
    char path[] = "/";
    char headers[] = "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/124.0.0.0 Safari/537.36\r\n"
                               "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*\r\n"
                               "Sec-Ch-Ua: \"Chromium\";v=\"124\", \"Google Chrome\";v=\"124\"\r\n"
                               "Sec-Ch-Ua-Platform: \"Windows\"\r\n"
                               "Connection: close\r\n\r\n";
    int timeout = 5000;
    SocksScan scan(domain, port, proxy_host, headers, proxy_port, timeout);
    SocksOutput result = scan.scan(path);
    cout << "target :- " << result.domain << endl;
    cout << "status code :- " << result.status_code << endl;
    cout << "latency_ms :- " << result.latency_ms << endl;
    cout << "reason phrase :- " << result.reason_phrase << endl;
    cout << "headers :- " << result.response_headers << endl;
}
