#pragma once
#include"http_scan.cpp"
#include"https_scan.cpp"

class UnifiedScanInterface{
private:
    char domain[256];
    char proto_port[128];
    char headers[8112];
public:
    UnifiedScanInterface(char _domain[256], char _proto_port[128], char _headers[8112]){
        strncpy(domain, _domain, 256);
        strncpy(headers, _headers, 8112);
        strncpy(proto_port, _proto_port, 128);
    }
    ScanOutput scan(char path[2048], int &sock, SSL *&target_ssl){
        ScanOutput result;
        if(strcmp(proto_port, "443") == 0 || strcmp(proto_port, "https")){
            GenericHttpsScan https(domain, headers);
            result = https.scan(path, sock, target_ssl);
            return result;
        } else if(strcmp(proto_port, "80") == 0 || strcmp(proto_port, "http")){
            GenericHttpScan http(domain, headers);
            result = http.http_scan(path, sock);
        }
        return result;
    }
};
