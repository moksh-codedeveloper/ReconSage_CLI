#include <vector>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <iostream>
using namespace std;

class LatencyCompiler
{
private:
    vector<double> latencyList;
    char domain[256];

public:
    LatencyCompiler(char _domain[256], vector<double> _latencyList)
    {
        strncpy(domain, _domain, 256);
        latencyList = _latencyList;
    }

    vector<uint64_t> Compile()
    {
        vector<uint64_t> hex_arr;

        for (const double &latency : latencyList)
        {
            uint64_t hex_token;
            // Safely copy all 8 bytes of the double into the 8-byte uint64_t
            memcpy(&hex_token, &latency, sizeof(double));

            cout << "Latency: " << latency
                 << " -> Compiled Hex64: 0x" << std::hex << hex_token << std::dec << endl;

            hex_arr.push_back(hex_token);
        }
        return hex_arr; // Added missing return statement!
    }

    // Fixed: Calculates true mathematical mean first, then emits the final compiled token
    uint64_t CompileMeanLatency()
    {
        if (latencyList.empty())
            return 0U;

        double total = 0.0;
        for (const double &latency : latencyList)
        {
            total += latency;
        }

        double mean = total / latencyList.size();

        uint64_t mean_hex_token;
        memcpy(&mean_hex_token, &mean, sizeof(double));

        return mean_hex_token;
    }
};