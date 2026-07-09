#include<vector>
#include <cstdint>
using namespace std;

#pragma pack(push, 1)
struct Compiler_Struct{
    char domain[256];
    vector<int> status_code_arr;
    vector<uint16_t> hex_status_code_arr;
    vector<uint16_t> common_codes_hex;
    vector<uint16_t> fallback_trackable_codes;
    vector<uint16_t> exotic_codes;
};
#pragma pack(pop)


