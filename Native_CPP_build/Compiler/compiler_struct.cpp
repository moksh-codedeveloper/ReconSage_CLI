#include<vector>
#include <cstdint>
using namespace std;

#pragma once

struct Compiler_Struct{
    char domain[256];
    vector<int> status_code_arr;
    vector<uint16_t> hex_status_code_arr;
    vector<uint16_t> common_codes_hex;
    vector<uint16_t> fallback_trackable_codes;
    vector<uint16_t> exotic_codes;
};


// The unified storage structure for latency data metrics
struct Latency_Compiler_Struct {
    char domain[256];
    vector<double> raw_latency_arr;
    vector<float> normalized_latency_arr; // Scaled between 0.0f and 1.0f
    vector<double> fast_responses;        // Under 200ms (Direct/Good Proxy)
    vector<double> medium_responses;      // 200ms - 1000ms (Average Tor/Proxy hop)
    vector<double> slow_or_timeout;       // Over 1000ms (Lagging or active block)
    double mean_latency;
};
