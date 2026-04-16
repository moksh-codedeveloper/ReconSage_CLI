#include <cstdint>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <chrono>
using namespace std;

struct ScanEngineModel
{
    char target[357];
    int status_code;
    char* response_headers;
    char reason_phrase[256];
    double latency_ms;
    ScanEngineModel() : status_code(0), latency_ms(0), response_headers(nullptr)
    {
        memset(target, 0, 357);
        memset(reason_phrase, 0, 256);
    }
};

class ScanEngine
{
private:
    struct timeval tv;

public:
    ScanEngine(int timeout)
    {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
    }

    ScanEngineModel* normalScan(char target[256], char path[100], char port[128])
    {
        ScanEngineModel* scan = new ScanEngineModel();
        snprintf(scan->target, sizeof(scan->target), "%s%s", target, path);
        struct addrinfo hints{}, *res;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(target, port, &hints, &res) != 0)
            return scan;
        
        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
            scan->status_code = -1;
            close(sock);
            freeaddrinfo(res);
            return scan;
        }
        
        char request[512];
        int req_len = snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, target);
        
        auto start = chrono::high_resolution_clock::now();
        send(sock, request, req_len, 0);
        size_t capacity = 65536;
        char* buff = (char*)malloc(capacity);
        int total_recieved = 0;
        int bytes;
        while((bytes = recv(sock, buff + total_recieved, capacity - total_recieved - 1, 0)) > 0){
            total_recieved += bytes;
            buff[total_recieved] = '\0';
            char* divider = strstr(buff, "\r\n\r\n");
            if(divider){
                size_t header_len = divider - buff;
                scan->response_headers = (char*)malloc(header_len + 1);
                memcpy(scan->response_headers, buff, header_len);
                scan->response_headers[header_len] = '\0';
                sscanf(scan->response_headers, "HTTP/%*f %d %255[^\r\n]", &scan->status_code, scan->reason_phrase);
                break;
            }
        }
        auto end = chrono::high_resolution_clock::now();
        scan->latency_ms = chrono::duration<double, milli>(end - start).count();
        free(buff);
        close(sock);
        freeaddrinfo(res);
        return scan;
    }
};


extern "C" {
    void* CreateEngine(int timeoutms){return new ScanEngine(timeoutms);}
    ScanEngineModel* PerformScan(void* engine, char target[256], char path[100], char port[128]){
        return static_cast<ScanEngine*>(engine)->normalScan(target, path, port);
    }
    void DestroyResult(ScanEngineModel* res) {
        if (res) {
            if (res->response_headers) free(res->response_headers);
            delete res;
        }
    }
}