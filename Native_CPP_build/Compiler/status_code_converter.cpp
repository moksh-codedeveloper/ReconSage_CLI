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
#include <vector>
#include <cstdint>
#include <iostream>
#include <cstring>
#include "compiler_struct.cpp"

using namespace std;

class StatusCodeCompiler
{
private:
    vector<int> status_code_arr;
    char domain[256];
    vector<int> CommonCodes = {200, 204, 201, 301, 302, 400, 401, 404, 403, 429, 500, 503, 0, 999};
    vector<int> StatusCodesFallback = {207, 422, 507, 307, 308, 407, 451, 444, 499, 521};

    uint16_t codes_to_hex(int code)
    {
        if (code == CommonCodes[0])
            return 0x20;
        if (code == CommonCodes[1])
            return 0x24;
        if (code == CommonCodes[2])
            return 0x21;
        if (code == CommonCodes[3])
            return 0x31;
        if (code == CommonCodes[4])
            return 0x32;
        if (code == CommonCodes[5])
            return 0x40;
        if (code == CommonCodes[6])
            return 0x41;
        if (code == CommonCodes[7])
            return 0x44;
        if (code == CommonCodes[8])
            return 0x43;
        if (code == CommonCodes[9])
            return 0x49;
        if (code == CommonCodes[10])
            return 0x50;
        if (code == CommonCodes[11])
            return 0x53;
        if (code == CommonCodes[12])
            return 0x00;
        if (code == CommonCodes[13])
            return 0x99;
        return 0x0FF;
    }

    uint16_t adv_codes_to_hex(int code)
    {
        if (code == StatusCodesFallback[0])
            return 0x27;
        if (code == StatusCodesFallback[1])
            return 0x47;
        if (code == StatusCodesFallback[2])
            return 0x57;
        if (code == StatusCodesFallback[3])
            return 0x37;
        if (code == StatusCodesFallback[4])
            return 0x38;
        if (code == StatusCodesFallback[5])
            return 0x47;
        if (code == StatusCodesFallback[6])
            return 0x45;
        if (code == StatusCodesFallback[7])
            return 0x44;
        if (code == StatusCodesFallback[8])
            return 0x49;
        if (code == StatusCodesFallback[9])
            return 0x52;
        return 0x0FF;
    }

    uint16_t other_no_codes_match_to_hex(int code)
    {
        int http_class = code / 100;
        if (http_class == 1)
            return 0x1FF;
        if (http_class == 2)
            return 0x2FF;
        if (http_class == 3)
            return 0x3FF;
        if (http_class == 4)
            return 0X4FF;
        if (http_class == 5)
            return 0x5FF;
        return 0x9FF;
    }

    // Your brilliant fixed validation loop
    vector<uint16_t> arr_codes_to_hex()
    {
        vector<uint16_t> hex_arr;
        for (const int &code : status_code_arr)
        {
            uint16_t hex = codes_to_hex(code);
            if (hex == 0x0FF)
            {
                uint16_t adv_hex = adv_codes_to_hex(code);
                if (adv_hex == 0x0FF)
                {
                    uint16_t other_hex = other_no_codes_match_to_hex(code);
                    hex_arr.push_back(other_hex);
                }
                else
                {
                    hex_arr.push_back(adv_hex);
                }
            }
            else
            {
                hex_arr.push_back(hex);
            }
        }
        return hex_arr;
    }

public:
    StatusCodeCompiler(vector<int> _status_code_arr, char _domain[256])
    {
        strncpy(domain, _domain, 256);
        status_code_arr = _status_code_arr;
    }

    // Master compilation handler that organizes the packed datasets
    Compiler_Struct Compile()
    {
        Compiler_Struct compiler_struct;

        // 1. Copy structural contexts
        strncpy(compiler_struct.domain, domain, 256);
        compiler_struct.status_code_arr = status_code_arr;

        // 2. Generate the complete master token layout
        compiler_struct.hex_status_code_arr = arr_codes_to_hex();

        // 3. Sort tokens into discrete categorical buckets for deep analysis
        for (size_t i = 0; i < status_code_arr.size(); ++i)
        {
            int original_code = status_code_arr[i];
            uint16_t compiled_hex = compiler_struct.hex_status_code_arr[i];

            // Check Ring 1 (Common)
            uint16_t test_common = codes_to_hex(original_code);
            if (test_common != 0x0FF)
            {
                compiler_struct.common_codes_hex.push_back(compiled_hex);
                continue;
            }

            // Check Ring 2 (Advanced Fallback)
            uint16_t test_adv = adv_codes_to_hex(original_code);
            if (test_adv != 0x0FF)
            {
                compiler_struct.fallback_trackable_codes.push_back(compiled_hex);
                continue;
            }

            // Otherwise, it landed in Ring 3 (Exotic Wildcard)
            compiler_struct.exotic_codes.push_back(compiled_hex);
        }

        return compiler_struct;
    }
};