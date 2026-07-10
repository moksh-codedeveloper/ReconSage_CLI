#include <vector>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <cmath>
#include "compiler_struct.cpp"
using namespace std;

class LatencyCompiler {
private:
    vector<double> latency_list;
    char domain[256];
    const double MAX_LATENCY_CAP = 5000.0; // 5 seconds max tracking window

public:
    LatencyCompiler(vector<double> _latency_list, char _domain[256]) {
        strncpy(domain, _domain, 256);
        latency_list = _latency_list;
    }

    // Normalizes a single raw latency value down to a 0.0f - 1.0f range
    float NormalizeLatency(double raw_ms) {
        if (raw_ms < 0.0) raw_ms = 0.0;
        if (raw_ms > MAX_LATENCY_CAP) raw_ms = MAX_LATENCY_CAP;
        return static_cast<float>(raw_ms / MAX_LATENCY_CAP);
    }

    // Master execution block that computes metrics and populates the struct
    Latency_Compiler_Struct Compile() {
        Latency_Compiler_Struct out_struct;
        
        // 1. Copy over basic platform tracking attributes
        strncpy(out_struct.domain, domain, 256);
        out_struct.raw_latency_arr = latency_list;

        double total_sum = 0.0;

        // 2. Continuous linear pass to categorize and normalize the execution time
        for (const double& ms : latency_list) {
            total_sum += ms;

            // Normalize and store the mathematical feature token
            out_struct.normalized_latency_arr.push_back(NormalizeLatency(ms));

            // Run structural categorization thresholds
            if (ms < 200.0) {
                out_struct.fast_responses.push_back(ms);
            } 
            else if (ms >= 200.0 && ms <= 1000.0) {
                out_struct.medium_responses.push_back(ms);
            } 
            else {
                out_struct.slow_or_timeout.push_back(ms);
            }
        }

        // 3. Mathematical Mean Calculation (Safely avoiding divide-by-zero)
        out_struct.mean_latency = latency_list.empty() ? 0.0 : (total_sum / latency_list.size());

        return out_struct;
    }
};

// // Add these public methods inside your LatencyCompiler class:

// double CalculateMean() {
//     if (latency_list.empty()) return 0.0;
//     double sum = 0.0;
//     for (const double& ms : latency_list) sum += ms;
//     return sum / latency_list.size();
// }

// double CalculateStdDev(double mean) {
//     if (latency_list.size() < 2) return 0.0; // Variance requires at least 2 points
    
//     double variance_sum = 0.0;
//     for (const double& ms : latency_list) {
//         double deviation = ms - mean;
//         variance_sum += (deviation * deviation); // Deviation squared
//     }
    
//     double variance = variance_sum / latency_list.size();
//     return sqrt(variance); // Standard Deviation (sigma)
// }

// // Establishes the dynamic wall where a request transitions from "normal" to "anomalous lag"
// double CalculateAnomalyThreshold(double mean, double std_dev) {
//     // Threshold = Mean + (K * StdDev)
//     // K = 2.0 captures 95% of normal traffic. Anything above this is statistically an anomaly (WAF throttle)
//     const double K_FACTOR = 2.0; 
//     return mean + (K_FACTOR * std_dev);
// }