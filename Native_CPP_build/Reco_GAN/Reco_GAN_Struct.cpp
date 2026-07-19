#include <vector>
using namespace std;

#pragma once

struct TelemetryTensor{
    double statusCode;
    double LatencyMS;
    vector<double> tokens;
};

struct TelemetryProcessedData{
    vector<double> status_code;
    vector<double> latency;
    vector<vector<double>> char_tokens;
};