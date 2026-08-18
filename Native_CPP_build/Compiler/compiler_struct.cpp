/*
 * ReconSage_Cli - Advanced Network & Telemetry Reconnaissance Framework
 * Copyright (C) 2026 ReconSage_Cli Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
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
};
