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
#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <chrono>
#include <cstring>
using namespace std;

inline vector<uint8_t> encode_dns_name(string domain)
{
    vector<uint8_t> encoded;
    string label = "";
    string target = domain + "."; // Append terminal dot to capture trailing label uniformly

    for (char c : target)
    {
        if (c == '.')
        {
            if (!label.empty())
            {
                encoded.push_back(static_cast<uint8_t>(label.length())); // Length octet
                for (char lc : label)
                {
                    encoded.push_back(static_cast<uint8_t>(lc)); // Label characters
                }
                label.clear();
            }
        }
        else
        {
            label += c;
        }
    }
    encoded.push_back(0); // Explicitly terminate with null label (0x00)
    return encoded;
}

inline int skip_name_field(const uint8_t *buffer, int offset, int buffer_len)
{
    while (offset < buffer_len)
    {
        uint8_t len = buffer[offset];

        // Check if high 2 bits match binary 11 (0xC0) - Indicates a compression pointer
        if ((len & 0xC0) == 0xC0)
        {
            return offset + 2; // Pointers are always exactly 2 bytes long
        }

        // Terminal null byte detected
        if (len == 0)
        {
            return offset + 1;
        }

        offset += (len + 1); // Move past length octet + label string length
    }
    return offset;
}

inline uint16_t generate_unique_run_id()
{
    auto now = chrono::high_resolution_clock::now();
    uint64_t microseconds = chrono::duration_cast<chrono::microseconds>(
                                now.time_since_epoch())
                                .count();
    uint16_t lower_bits = static_cast<uint16_t>(microseconds & 0xFFFF);
    uint16_t upper_bits = static_cast<uint16_t>((microseconds >> 16) & 0xFFFF);

    uint16_t execution_id = lower_bits ^ upper_bits;
    if (execution_id == 0)
    {
        execution_id = 0x4A4A;
    }

    return execution_id;
}

inline int extract_status_from_buffer(char buff[65536])
{
    if (!buff || buff[0] == '\0')
        return -1;

    // Look for the last HTTP occurrence to skip past proxy tunnel acknowledgments
    const char *target = strstr(buff, "HTTP/");
    const char *next_match = target;

    while (next_match != nullptr)
    {
        next_match = strstr(target + 5, "HTTP/");
        if (next_match != nullptr)
        {
            target = next_match;
        }
    }

    // Advance tracker past the standard "HTTP/1.1 " header marker bounds
    const char *ptr = strchr(target, ' ');
    if (!ptr)
        return -1;
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