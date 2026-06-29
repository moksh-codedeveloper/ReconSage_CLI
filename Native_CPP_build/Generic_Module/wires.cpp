#include <cstdint>
#include <vector>
#include <string>
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