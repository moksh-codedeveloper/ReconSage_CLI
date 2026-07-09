#include<vector>
#include<cstdint>
#include<iostream>
#include<cstring>
using namespace std;

struct Compiler_Struct{
    char domain[256];
    vector<int> status_code_arr;
    vector<uint16_t> hex_status_code_arr;
    vector<uint16_t> common_codes_hex;
    vector<uint16_t> fallback_trackable_codes;
    vector<uint16_t> exotic_codes;
};

class StatusCodeCompiler{
private:
    vector<int> status_code_arr;
    char domain[256];
    // uint16_t unknown = 0x00;
    // uint16_t info = 0x01;
    // uint16_t success = 0x02;
    // uint16_t route = 0x03;
    // uint16_t err = 0x04;
    // uint16_t server_err = 0x05;
    // uint16_t network_err = 0x09;
    vector<int> CommonCodes = {200, 204, 201, 301, 302, 400, 401, 404, 403, 429, 500, 503, 0, 999};
public:
    StatusCodeCompiler(vector<int> _status_code_arr, char _domain[256]){
        strncpy(domain, _domain, 256);
        status_code_arr = _status_code_arr;
    }
    uint16_t codes_to_hex(int code){
        if(code == CommonCodes[0]) return 0x20;
        if(code == CommonCodes[1]) return 0x24;
        if(code == CommonCodes[2]) return 0x21;
        if(code == CommonCodes[3]) return 0x31;
        if(code == CommonCodes[4]) return 0x32;
        if(code == CommonCodes[5]) return 0x40;
        if(code == CommonCodes[6]) return 0x41;
        if(code == CommonCodes[7]) return 0x44;
        if(code == CommonCodes[8]) return 0x43;
        if(code == CommonCodes[9]) return 0x49;
        if(code == CommonCodes[10]) return 0x50;
        if(code == CommonCodes[11]) return 0x53;
        if(code == CommonCodes[12]) return 0x00;
        if(code == CommonCodes[13]) return 0x99;
        return 0x0;
    }
    vector<uint16_t> arr_codes_to_hex(){
        vector<uint16_t> hex_arr;
        for(const int &code : status_code_arr){
            uint16_t hex = codes_to_hex(code);
            hex_arr.push_back(hex);
        }
        return hex_arr;
    }
    Compiler_Struct Compile(){
        Compiler_Struct compiler_struct;
        return compiler_struct;
    }
};