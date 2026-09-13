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
#include "compiler_struct.hpp"

using namespace std;

class StatusCodeCompiler
{
private:
    vector<int> status_code_arr;
    char domain[256];
    vector<int> CommonCodes = {200, 204, 201, 301, 302, 400, 401, 404, 403, 429, 500, 503, 0, 999};
    vector<int> StatusCodesFallback = {207, 422, 507, 307, 308, 407, 451, 444, 499, 521};
    uint16_t codes_to_hex(int code);
    uint16_t adv_codes_to_hex(int code);
    uint16_t other_no_codes_match_to_hex(int code);
    vector<uint16_t> arr_codes_to_hex();

public:
    StatusCodeCompiler(vector<int> _status_code_arr, char _domain[256]);
    Compiler_Struct Compile();
};